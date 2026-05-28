#include "app_gpio.h"

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lvgl/lvgl.h"

#define GPIO_SYSFS_ROOT "/sys/class/gpio"
#define UART_DEVICE_PATH "/dev/ttyS0"

typedef struct {
    const char *label;
    int sysfs_num;
} gpio_output_t;

typedef struct {
    uint32_t id;
    bool on;
} gpio_button_sync_t;

static const gpio_output_t g_gpio_outputs[APP_GPIO_OUTPUT_COUNT] = {
    [APP_GPIO_VALVE1]  = { "VALVE1", 139 },
    [APP_GPIO_VALVE2]  = { "VALVE2", 140 },
    [APP_GPIO_VALVE3]  = { "VALVE3", 141 },
    [APP_GPIO_MAGNET]  = { "MAGNET", 142 },
    [APP_GPIO_STEPPER] = { "STEPPER", 234 },
    [APP_GPIO_CAMERA]  = { "CAMERA", -1 },
};

static pthread_mutex_t g_gpio_lock = PTHREAD_MUTEX_INITIALIZER;
static int g_gpio_value_fds[APP_GPIO_OUTPUT_COUNT] = { -1, -1, -1, -1, -1, -1 };
static bool g_gpio_state[APP_GPIO_OUTPUT_COUNT];
static lv_obj_t *g_gpio_buttons[APP_GPIO_OUTPUT_COUNT];
static bool g_ignore_gpio_events[APP_GPIO_OUTPUT_COUNT];
static app_gpio_button_handler_t g_button_handler;
static void *g_button_handler_user_data;

static const char *getenv_default(const char *name, const char *dflt)
{
    const char *value = getenv(name);

    return value != NULL ? value : dflt;
}

static void gpio_build_node_path(char *buf, size_t buf_size, int sysfs_num, const char *node)
{
    snprintf(buf, buf_size, GPIO_SYSFS_ROOT "/gpio%d/%s", sysfs_num, node);
}

static int gpio_write_text_file(const char *path, const char *text)
{
    size_t len = strlen(text);
    int fd = open(path, O_WRONLY | O_CLOEXEC);

    if (fd < 0) return -1;

    while (len > 0) {
        ssize_t written = write(fd, text, len);
        if (written > 0) {
            text += written;
            len -= (size_t)written;
            continue;
        }
        if (written < 0 && errno == EINTR) continue;

        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

static bool gpio_output_conflicts_with_uart(uint32_t id)
{
    const char *uart_device = getenv_default("LV_UART_DEVICE", UART_DEVICE_PATH);

    return id == APP_GPIO_STEPPER && strcmp(uart_device, UART_DEVICE_PATH) == 0;
}

static int gpio_write_value_fd(int fd, bool enable)
{
    char value = enable ? '1' : '0';

    if (lseek(fd, 0, SEEK_SET) < 0) return -1;
    if (write(fd, &value, 1) != 1) return -1;

    return 0;
}

static int gpio_prepare_output(uint32_t id)
{
    const gpio_output_t *output;
    char gpio_dir[64];
    char direction_path[96];
    char value_path[96];
    char export_num[16];
    int fd;
    int retry;

    if (id >= APP_GPIO_OUTPUT_COUNT) return -1;

    output = &g_gpio_outputs[id];
    if (output->sysfs_num < 0) return -1;
    if (gpio_output_conflicts_with_uart(id)) {
        fprintf(stderr, "[gpio] skip %s: sysfs %d conflicts with UART0 on PH10\n",
                output->label, output->sysfs_num);
        return -1;
    }

    snprintf(gpio_dir, sizeof(gpio_dir), GPIO_SYSFS_ROOT "/gpio%d", output->sysfs_num);
    if (access(gpio_dir, F_OK) != 0) {
        snprintf(export_num, sizeof(export_num), "%d", output->sysfs_num);
        if (gpio_write_text_file(GPIO_SYSFS_ROOT "/export", export_num) != 0 && errno != EBUSY) {
            fprintf(stderr, "[gpio] export gpio%d failed: %s\n",
                    output->sysfs_num, strerror(errno));
            return -1;
        }

        for (retry = 0; retry < 20; retry++) {
            if (access(gpio_dir, F_OK) == 0) break;
            usleep(10000);
        }
    }

    if (access(gpio_dir, F_OK) != 0) {
        fprintf(stderr, "[gpio] gpio%d path not ready after export\n", output->sysfs_num);
        return -1;
    }

    gpio_build_node_path(direction_path, sizeof(direction_path), output->sysfs_num, "direction");
    if (gpio_write_text_file(direction_path, "out") != 0) {
        fprintf(stderr, "[gpio] set gpio%d direction failed: %s\n",
                output->sysfs_num, strerror(errno));
        return -1;
    }

    gpio_build_node_path(value_path, sizeof(value_path), output->sysfs_num, "value");
    fd = open(value_path, O_WRONLY | O_CLOEXEC);
    if (fd < 0) {
        fprintf(stderr, "[gpio] open gpio%d value failed: %s\n",
                output->sysfs_num, strerror(errno));
        return -1;
    }

    if (gpio_write_value_fd(fd, false) != 0)
        fprintf(stderr, "[gpio] initialize gpio%d low failed: %s\n",
                output->sysfs_num, strerror(errno));

    printf("[gpio] ready %s via /sys/class/gpio/gpio%d\n",
           output->label, output->sysfs_num);
    return fd;
}

static void gpio_control(uint32_t id, bool enable)
{
    const gpio_output_t *output;
    int fd;
    int rc;

    if (id >= APP_GPIO_OUTPUT_COUNT) {
        printf("Invalid GPIO id: %u\n", id);
        return;
    }

    output = &g_gpio_outputs[id];
    if (output->sysfs_num < 0) {
        printf("[gpio] %s has no sysfs mapping\n", output->label);
        return;
    }

    pthread_mutex_lock(&g_gpio_lock);
    fd = g_gpio_value_fds[id];
    if (fd < 0) {
        fd = gpio_prepare_output(id);
        g_gpio_value_fds[id] = fd;
    }
    if (fd < 0) {
        pthread_mutex_unlock(&g_gpio_lock);
        printf("gpio_control: %s not initialized\n", output->label);
        return;
    }

    rc = gpio_write_value_fd(fd, enable);
    if (rc == 0) g_gpio_state[id] = enable;
    pthread_mutex_unlock(&g_gpio_lock);

    if (rc != 0)
        perror("write gpio control failed");
    else
        printf("[gpio] %s (sysfs %d) %s\n",
               output->label, output->sysfs_num, enable ? "ON" : "OFF");
}

static void gpio_sync_button_state_async(void *user_data)
{
    gpio_button_sync_t *sync = user_data;
    lv_obj_t *btn;

    if (sync == NULL) return;
    if (sync->id >= APP_GPIO_OUTPUT_COUNT) {
        free(sync);
        return;
    }

    btn = g_gpio_buttons[sync->id];
    if (btn != NULL) {
        g_ignore_gpio_events[sync->id] = true;
        if (sync->on) lv_obj_add_state(btn, LV_STATE_CHECKED);
        else          lv_obj_clear_state(btn, LV_STATE_CHECKED);
        g_ignore_gpio_events[sync->id] = false;
    }

    free(sync);
}

static void gpio_schedule_button_sync(uint32_t id, bool on)
{
    gpio_button_sync_t *sync;

    if (id >= APP_GPIO_OUTPUT_COUNT) return;

    sync = malloc(sizeof(*sync));
    if (sync == NULL) {
        fprintf(stderr, "[ui] failed to allocate button sync request\n");
        return;
    }

    sync->id = id;
    sync->on = on;
    if (lv_async_call(gpio_sync_button_state_async, sync) != LV_RESULT_OK) {
        fprintf(stderr, "[ui] failed to schedule button sync for %s\n",
                g_gpio_outputs[id].label);
        free(sync);
    }
}

static void gpio_btn_event_cb(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    uint32_t id = (uint32_t)(uintptr_t)lv_event_get_user_data(e);
    bool on = lv_obj_has_state(btn, LV_STATE_CHECKED);

    if (id >= APP_GPIO_OUTPUT_COUNT) return;
    if (g_ignore_gpio_events[id]) return;

    if (g_button_handler != NULL) {
        g_button_handler(id, on, g_button_handler_user_data);
        return;
    }

    app_gpio_apply_output(id, on, false);
}

void app_gpio_init(void)
{
    uint32_t i;

    for (i = 0; i < APP_GPIO_OUTPUT_COUNT; i++) {
        if (g_gpio_value_fds[i] >= 0 || g_gpio_outputs[i].sysfs_num < 0) continue;
        g_gpio_value_fds[i] = gpio_prepare_output(i);
        g_gpio_state[i] = false;
    }
}

void app_gpio_cleanup(void)
{
    uint32_t i;

    pthread_mutex_lock(&g_gpio_lock);
    for (i = 0; i < APP_GPIO_OUTPUT_COUNT; i++) {
        if (g_gpio_value_fds[i] >= 0) {
            close(g_gpio_value_fds[i]);
            g_gpio_value_fds[i] = -1;
        }
    }
    pthread_mutex_unlock(&g_gpio_lock);

    for (i = 0; i < APP_GPIO_OUTPUT_COUNT; i++) {
        g_gpio_buttons[i] = NULL;
        g_ignore_gpio_events[i] = false;
    }
}

void app_gpio_apply_output(uint32_t id, bool enable, bool sync_ui)
{
    if (id >= APP_GPIO_OUTPUT_COUNT) return;

    gpio_control(id, enable);
    if (sync_ui) gpio_schedule_button_sync(id, enable);
}

static const char *gpio_icons[APP_GPIO_OUTPUT_COUNT] = {
    [APP_GPIO_VALVE1]  = LV_SYMBOL_TINT,
    [APP_GPIO_VALVE2]  = LV_SYMBOL_TINT,
    [APP_GPIO_VALVE3]  = LV_SYMBOL_TINT,
    [APP_GPIO_MAGNET]  = LV_SYMBOL_CHARGE,
    [APP_GPIO_STEPPER] = LV_SYMBOL_SETTINGS,
    [APP_GPIO_CAMERA]  = LV_SYMBOL_IMAGE,
};

void app_gpio_create_button_panel(lv_obj_t *parent,
                                  app_gpio_button_handler_t handler,
                                  void *user_data)
{
    uint32_t i;

    g_button_handler           = handler;
    g_button_handler_user_data = user_data;

    /* Configure flex layout on the provided parent */
    lv_obj_set_layout(parent, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    /* Extra left pad (50px) so buttons are away from the physical touch dead zone
     * that exists on the left logical edge (= physical top edge after 90° rotation) */
    lv_obj_set_style_pad_left(parent, 50, 0);
    lv_obj_set_style_pad_right(parent, 16, 0);
    lv_obj_set_style_pad_top(parent, 16, 0);
    lv_obj_set_style_pad_bottom(parent, 16, 0);
    lv_obj_set_style_pad_column(parent, 16, 0);
    lv_obj_set_style_pad_row(parent, 16, 0);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    for (i = 0; i < APP_GPIO_OUTPUT_COUNT; i++) {
        lv_obj_t *btn = lv_button_create(parent);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(btn, 228, 168);

        /* OFF style */
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x0E1826), 0);
        lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(btn, lv_color_hex(0x1C3450), 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_radius(btn, 14, 0);
        lv_obj_set_style_shadow_width(btn, 0, 0);

        /* ON style — green glow */
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x0A2B1A), LV_STATE_CHECKED);
        lv_obj_set_style_border_color(btn, lv_color_hex(0x00FF94), LV_STATE_CHECKED);
        lv_obj_set_style_border_width(btn, 2, LV_STATE_CHECKED);
        lv_obj_set_style_shadow_color(btn, lv_color_hex(0x00FF94), LV_STATE_CHECKED);
        lv_obj_set_style_shadow_width(btn, 18, LV_STATE_CHECKED);
        lv_obj_set_style_shadow_spread(btn, 2, LV_STATE_CHECKED);
        lv_obj_set_style_shadow_opa(btn, LV_OPA_60, LV_STATE_CHECKED);

        /* Flex column inside */
        lv_obj_set_layout(btn, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_CENTER,
                              LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_all(btn, 12, 0);
        lv_obj_set_style_pad_row(btn, 10, 0);

        lv_obj_add_event_cb(btn, gpio_btn_event_cb, LV_EVENT_VALUE_CHANGED,
                            (void *)(uintptr_t)i);
        g_gpio_buttons[i] = btn;

        /* Icon */
        lv_obj_t *icon = lv_label_create(btn);
        lv_label_set_text(icon, gpio_icons[i]);
        lv_obj_set_style_text_font(icon, &lv_font_montserrat_36, 0);
        lv_obj_set_style_text_color(icon, lv_color_hex(0x00D4FF), 0);

        /* Label */
        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, g_gpio_outputs[i].label);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xA0C4DC), 0);

        if (g_gpio_state[i]) lv_obj_add_state(btn, LV_STATE_CHECKED);
    }
}