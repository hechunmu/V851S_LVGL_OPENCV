/*
 * stream_thread.cpp  –  MJPEG push relay for V851S
 *
 * The server cannot reach the board's private LAN IP (NAT).
 * This thread connects OUT to the relay server and pushes an
 * MJPEG stream over a persistent HTTP POST connection.
 *
 *   Board  →  POST http://RELAY_HOST:RELAY_API_PORT/api/v851s/stream
 *   Server →  pipe incoming body to  ffmpeg -f mjpeg -i pipe:0  → RTMP
 *
 * Also keeps a local MJPEG HTTP server on MJPEG_PORT for LAN clients.
 */

#include "stream_thread.h"
#include "cam_thread.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <jpeglib.h>
#include <setjmp.h>
#include <stdint.h>

/* ---- tunables ---- */
#define MJPEG_PORT       8090
#define JPEG_QUALITY     55
#define TARGET_FPS       15
#define RELAY_HOST       "159.75.115.208"
#define RELAY_TCP_PORT   8086   /* raw TCP stream port */
/* -------------------- */

static volatile int s_stop = 0;
static pthread_t    s_push_tid = 0;
static pthread_t    s_srv_tid  = 0;

/* ---- JPEG error handler ---- */
struct my_jpeg_err {
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buf;
};
static void jpeg_error_exit_fn(j_common_ptr cinfo)
{
    struct my_jpeg_err *e = (struct my_jpeg_err *)cinfo->err;
    longjmp(e->setjmp_buf, 1);
}

/* Encode CAM_W×CAM_H XRGB8888 → JPEG into *outbuf (caller frees).
 * Returns encoded size or 0 on error. */
static size_t encode_jpeg(const uint32_t *xrgb, unsigned char **outbuf)
{
    struct jpeg_compress_struct cinfo;
    struct my_jpeg_err jerr;
    unsigned long out_size = 0;

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = jpeg_error_exit_fn;
    if (setjmp(jerr.setjmp_buf)) {
        jpeg_destroy_compress(&cinfo);
        if (*outbuf) { free(*outbuf); *outbuf = NULL; }
        return 0;
    }

    jpeg_create_compress(&cinfo);

    *outbuf = NULL;
    jpeg_mem_dest(&cinfo, outbuf, &out_size);

    cinfo.image_width      = CAM_W;
    cinfo.image_height     = CAM_H;
    cinfo.input_components = 3;
    cinfo.in_color_space   = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, JPEG_QUALITY, TRUE);
    cinfo.dct_method = JDCT_FASTEST;
    jpeg_start_compress(&cinfo, TRUE);

    JSAMPLE row[CAM_W * 3];
    while (cinfo.next_scanline < cinfo.image_height) {
        const uint32_t *src = xrgb + cinfo.next_scanline * CAM_W;
        JSAMPLE *dst = row;
        for (int x = 0; x < CAM_W; x++) {
            uint32_t px = src[x];
            *dst++ = (px >> 16) & 0xFF;
            *dst++ = (px >>  8) & 0xFF;
            *dst++ = (px >>  0) & 0xFF;
        }
        JSAMPROW rp = row;
        jpeg_write_scanlines(&cinfo, &rp, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    return (size_t)out_size;
}

/* ---- write_all: retry on EINTR ---- */
static int write_all(int fd, const void *buf, size_t len)
{
    const char *p = (const char *)buf;
    while (len > 0) {
        ssize_t n = write(fd, p, len);
        if (n <= 0) {
            if (n < 0 && errno == EINTR) continue;
            return -1;
        }
        p += n; len -= n;
    }
    return 0;
}

/* ---- wait up to max_sec for wlan0 to get an IPv4 address ---- */
static int wait_for_ip(char *buf, size_t len, int max_sec)
{
    for (int i = 0; i < max_sec; i++) {
        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd < 0) { sleep(1); continue; }
        struct ifreq ifr;
        memset(&ifr, 0, sizeof ifr);
        strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ - 1);
        int ret = ioctl(fd, SIOCGIFADDR, &ifr);
        close(fd);
        if (ret == 0) {
            struct sockaddr_in *sa = (struct sockaddr_in *)&ifr.ifr_addr;
            const char *ip = inet_ntoa(sa->sin_addr);
            if (strcmp(ip, "0.0.0.0") != 0) {
                strncpy(buf, ip, len - 1);
                buf[len - 1] = '\0';
                return 0;
            }
        }
        sleep(1);
    }
    return -1;
}

/* ---- Push thread: board → relay server ---- */
static void *push_thread_func(void *)
{
    /* Wait for camera */
    for (int i = 0; i < 100 && !cam_is_open() && !s_stop; i++)
        usleep(100000);

    if (!cam_is_open()) {
        fprintf(stderr, "[stream] camera not ready, push disabled\n");
        return NULL;
    }

    char myip[64] = "";
    if (wait_for_ip(myip, sizeof myip, 90) != 0) {
        fprintf(stderr, "[stream] wlan0 has no IP, push disabled\n");
        return NULL;
    }
    fprintf(stderr, "[stream] wlan0 IP: %s — waiting for camON\n", myip);

    static uint32_t pixbuf[CAM_H * CAM_W];

    while (!s_stop) {
        /* Wait until the user presses camON */
        if (!cam_is_capturing()) {
            usleep(200000);
            continue;
        }

        /* Connect to relay server raw TCP port */
        int s = socket(AF_INET, SOCK_STREAM, 0);
        if (s < 0) { sleep(5); continue; }

        struct timeval tv;
        tv.tv_sec = 10; tv.tv_usec = 0;
        setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof tv);

        struct sockaddr_in srv;
        memset(&srv, 0, sizeof srv);
        srv.sin_family = AF_INET;
        srv.sin_port   = htons(RELAY_TCP_PORT);
        inet_pton(AF_INET, RELAY_HOST, &srv.sin_addr);

        if (connect(s, (struct sockaddr*)&srv, sizeof srv) != 0) {
            fprintf(stderr, "[stream] connect relay failed: %s, retry in 5s\n",
                    strerror(errno));
            close(s);
            sleep(5);
            continue;
        }

        fprintf(stderr, "[stream] pushing raw JPEG → %s:%d\n",
                RELAY_HOST, RELAY_TCP_PORT);

        unsigned char *jpg = NULL;
        int ok = 1;
        struct timeval last_send = {0, 0};
        const long long frame_us = 1000000 / TARGET_FPS;
        int seq = 0;

        while (!s_stop && ok && cam_is_capturing()) {
            if (!cam_copy_frame(pixbuf, &seq)) {
                usleep(10000);
                continue;
            }

            /* Rate limit */
            struct timeval now;
            gettimeofday(&now, NULL);
            long long elapsed = (long long)(now.tv_sec - last_send.tv_sec) * 1000000LL
                             + (now.tv_usec - last_send.tv_usec);
            if (elapsed < frame_us) {
                usleep(frame_us - elapsed);
                continue;
            }
            last_send = now;

            size_t jsz = encode_jpeg(pixbuf, &jpg);
            if (!jsz || !jpg) { usleep(10000); continue; }

            if (write_all(s, jpg, jsz) < 0) {
                fprintf(stderr, "[stream] write failed errno=%d\n", errno);
                ok = 0;
            }

            free(jpg); jpg = NULL;
        }

        if (jpg) { free(jpg); jpg = NULL; }
        close(s);

        if (!s_stop) {
            if (!cam_is_capturing())
                fprintf(stderr, "[stream] camOFF — stream stopped\n");
            else
                fprintf(stderr, "[stream] relay connection lost, retry in 5s\n");
            sleep(5);
        }
    }
    return NULL;
}

/* ---- Local MJPEG HTTP server (serve LAN clients) ---- */
static void serve_client(int cfd)
{
    char rbuf[512];
    recv(cfd, rbuf, sizeof rbuf - 1, 0);

    const char *resp_hdr =
        "HTTP/1.0 200 OK\r\n"
        "Content-Type: multipart/x-mixed-replace; boundary=mjpegbnd\r\n"
        "Cache-Control: no-cache\r\n"
        "Connection: close\r\n\r\n";
    write(cfd, resp_hdr, strlen(resp_hdr));

    static uint32_t pixbuf[CAM_H * CAM_W];
    int seq = 0;
    unsigned char *jpg = NULL;
    char hdr[256];

    fprintf(stderr, "[stream] local client fd=%d connected\n", cfd);

    while (!s_stop) {
        if (!cam_copy_frame(pixbuf, &seq)) { usleep(30000); continue; }

        size_t jsz = encode_jpeg(pixbuf, &jpg);
        if (!jsz || !jpg) { usleep(30000); continue; }

        int hlen = snprintf(hdr, sizeof hdr,
            "--mjpegbnd\r\n"
            "Content-Type: image/jpeg\r\n"
            "Content-Length: %zu\r\n\r\n", jsz);

        if (write_all(cfd, hdr, hlen) < 0) break;
        if (write_all(cfd, jpg, jsz)  < 0) break;
        if (write_all(cfd, "\r\n", 2) < 0) break;

        free(jpg); jpg = NULL;
    }

    if (jpg) free(jpg);
    close(cfd);
    fprintf(stderr, "[stream] local client fd=%d disconnected\n", cfd);
}

static void *srv_thread_func(void *)
{
    for (int i = 0; i < 100 && !cam_is_open() && !s_stop; i++)
        usleep(100000);
    if (!cam_is_open()) return NULL;

    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) return NULL;

    int yes = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(MJPEG_PORT);

    if (bind(sfd, (struct sockaddr*)&addr, sizeof addr) < 0) {
        perror("[stream] bind"); close(sfd); return NULL;
    }
    listen(sfd, 2);
    fcntl(sfd, F_SETFL, O_NONBLOCK);
    fprintf(stderr, "[stream] local MJPEG server on port %d\n", MJPEG_PORT);

    while (!s_stop) {
        int cfd = accept(sfd, NULL, NULL);
        if (cfd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) { usleep(50000); continue; }
            break;
        }
        serve_client(cfd);
    }
    close(sfd);
    return NULL;
}

static void crash_handler(int sig)
{
    const char *msg = "[stream] FATAL: received signal (crash)\n";
    write(2, msg, 40);
    _exit(128 + sig);
}

void stream_thread_start(void)
{
    signal(SIGPIPE, SIG_IGN);
    signal(SIGHUP,  SIG_IGN);   /* survive ADB/terminal disconnect */
    signal(SIGSEGV, crash_handler);
    signal(SIGBUS,  crash_handler);
    signal(SIGABRT, crash_handler);
    s_stop = 0;
    pthread_create(&s_push_tid, NULL, push_thread_func, NULL);
    pthread_create(&s_srv_tid,  NULL, srv_thread_func,  NULL);
}

void stream_thread_stop(void)
{
    s_stop = 1;
    if (s_push_tid) { pthread_join(s_push_tid, NULL); s_push_tid = 0; }
    if (s_srv_tid)  { pthread_join(s_srv_tid,  NULL); s_srv_tid  = 0; }
}
