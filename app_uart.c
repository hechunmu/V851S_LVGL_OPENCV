#include "app_uart.h"

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

#define UART_DEVICE_PATH "/dev/ttyS0"
#define UART_DEFAULT_BAUD 115200L

static int g_uart_fd = -1;
static volatile sig_atomic_t *g_running_flag;
static pthread_t g_uart_thread;
static bool g_uart_thread_started = false;
static pthread_mutex_t g_uart_lock = PTHREAD_MUTEX_INITIALIZER;
static app_uart_frame_handler_t g_frame_handler;
static void *g_frame_handler_user_data;

static const char *getenv_default(const char *name, const char *dflt)
{
    const char *value = getenv(name);

    return value != NULL ? value : dflt;
}

static void log_hex_frame(const char *prefix, const uint8_t *data, size_t len)
{
    size_t i;

    printf("%s", prefix);
    for (i = 0; i < len; i++) printf("%s%02X", i == 0 ? "" : " ", data[i]);
    printf("\n");
}

static speed_t uart_baud_to_speed(long baud)
{
    switch (baud) {
    case 9600:   return B9600;
    case 19200:  return B19200;
    case 38400:  return B38400;
    case 57600:  return B57600;
    case 115200: return B115200;
    default:     return 0;
    }
}

static int uart_open_port(void)
{
    const char *device = getenv_default("LV_UART_DEVICE", UART_DEVICE_PATH);
    const char *baud_env = getenv("LV_UART_BAUD");
    char *endptr = NULL;
    long baud = UART_DEFAULT_BAUD;
    speed_t speed;
    struct termios tty;
    int fd;

    if (baud_env && *baud_env != '\0') {
        baud = strtol(baud_env, &endptr, 10);
        if (endptr == baud_env || *endptr != '\0') {
            fprintf(stderr, "[uart] invalid LV_UART_BAUD=%s, fallback to %ld\n",
                    baud_env, UART_DEFAULT_BAUD);
            baud = UART_DEFAULT_BAUD;
        }
    }

    speed = uart_baud_to_speed(baud);
    if (speed == 0) {
        fprintf(stderr, "[uart] unsupported baud rate: %ld\n", baud);
        return -1;
    }

    fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0) {
        fprintf(stderr, "[uart] open %s failed: %s\n", device, strerror(errno));
        return -1;
    }

    if (tcgetattr(fd, &tty) != 0) {
        fprintf(stderr, "[uart] tcgetattr failed: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    cfmakeraw(&tty);
    tty.c_cflag |= CLOCAL | CREAD;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
#ifdef CRTSCTS
    tty.c_cflag &= ~CRTSCTS;
#endif
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 1;
    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        fprintf(stderr, "[uart] tcsetattr failed: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    if (tcflush(fd, TCIOFLUSH) != 0)
        fprintf(stderr, "[uart] tcflush failed: %s\n", strerror(errno));

    printf("[uart] opened %s @ %ld 8N1\n", device, baud);
    return fd;
}

static int uart_ensure_open(void)
{
    int fd;

    pthread_mutex_lock(&g_uart_lock);
    fd = g_uart_fd;
    pthread_mutex_unlock(&g_uart_lock);
    if (fd >= 0) return fd;

    fd = uart_open_port();
    if (fd < 0) return -1;

    pthread_mutex_lock(&g_uart_lock);
    if (g_uart_fd < 0) {
        g_uart_fd = fd;
    } else {
        close(fd);
        fd = g_uart_fd;
    }
    pthread_mutex_unlock(&g_uart_lock);

    return fd;
}

static void uart_mark_closed(int fd)
{
    pthread_mutex_lock(&g_uart_lock);
    if (g_uart_fd == fd) {
        close(g_uart_fd);
        g_uart_fd = -1;
    }
    pthread_mutex_unlock(&g_uart_lock);
}

static void uart_dispatch_frame(const uint8_t frame[4])
{
    log_hex_frame("[uart] RX ", frame, 4);

    if (g_frame_handler != NULL)
        g_frame_handler(frame, g_frame_handler_user_data);
}

static void uart_process_rx_bytes(const uint8_t *data, size_t len)
{
    static uint8_t frame[4];
    static size_t frame_len = 0;
    size_t i;

    for (i = 0; i < len; i++) {
        if (frame_len < sizeof(frame)) {
            frame[frame_len++] = data[i];
        } else {
            memmove(frame, frame + 1, sizeof(frame) - 1);
            frame[sizeof(frame) - 1] = data[i];
        }

        if (frame_len == sizeof(frame) && frame[0] == 0x55 && frame[1] == 0x43)
            uart_dispatch_frame(frame);
    }
}

static void *uart_rx_thread_main(void *arg)
{
    (void)arg;

    while (g_running_flag == NULL || *g_running_flag) {
        int fd = uart_ensure_open();
        fd_set readfds;
        struct timeval timeout;
        int ret;

        if (fd < 0) {
            sleep(1);
            continue;
        }

        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 500000;

        ret = select(fd + 1, &readfds, NULL, NULL, &timeout);
        if (ret < 0) {
            if (errno == EINTR) continue;
            fprintf(stderr, "[uart] select failed: %s\n", strerror(errno));
            uart_mark_closed(fd);
            continue;
        }
        if (ret == 0 || !FD_ISSET(fd, &readfds)) continue;

        for (;;) {
            uint8_t rx_buf[64];
            ssize_t read_len = read(fd, rx_buf, sizeof(rx_buf));

            if (read_len > 0) {
                uart_process_rx_bytes(rx_buf, (size_t)read_len);
                if (read_len == (ssize_t)sizeof(rx_buf)) continue;
                break;
            }
            if (read_len == 0) break;
            if (errno == EINTR) continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;

            fprintf(stderr, "[uart] read failed: %s\n", strerror(errno));
            uart_mark_closed(fd);
            break;
        }
    }

    return NULL;
}

void app_uart_init(volatile sig_atomic_t *running_flag,
                   app_uart_frame_handler_t frame_handler,
                   void *user_data)
{
    g_running_flag = running_flag;
    g_frame_handler = frame_handler;
    g_frame_handler_user_data = user_data;

    (void)uart_ensure_open();

    if (g_uart_thread_started) return;

    if (pthread_create(&g_uart_thread, NULL, uart_rx_thread_main, NULL) != 0) {
        fprintf(stderr, "[uart] pthread_create failed: %s\n", strerror(errno));
        return;
    }

    g_uart_thread_started = true;
}

void app_uart_cleanup(void)
{
    int fd;

    if (g_uart_thread_started) {
        pthread_join(g_uart_thread, NULL);
        g_uart_thread_started = false;
    }

    pthread_mutex_lock(&g_uart_lock);
    fd = g_uart_fd;
    g_uart_fd = -1;
    pthread_mutex_unlock(&g_uart_lock);

    if (fd >= 0) close(fd);
}

int app_uart_send_frame(const uint8_t *data, size_t len)
{
    int fd = uart_ensure_open();
    size_t offset = 0;

    if (fd < 0 || data == NULL || len == 0) return -1;

    log_hex_frame("[uart] TX ", data, len);
    while (offset < len) {
        ssize_t written = write(fd, data + offset, len - offset);
        if (written > 0) {
            offset += (size_t)written;
            continue;
        }
        if (written < 0 && errno == EINTR) continue;
        if (written < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            usleep(1000);
            continue;
        }

        fprintf(stderr, "[uart] write failed: %s\n", strerror(errno));
        uart_mark_closed(fd);
        return -1;
    }

    return 0;
}