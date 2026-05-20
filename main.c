#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"

#include "lv_100ask_lesson_demos/lv_100ask_lesson_demos.h"
#include "lv_port_indev.h"
#include "cam_thread.h"

#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/types.h>

/* ------------------------------------------------------------------ */
/*  GPIO control                                                        */
/* ------------------------------------------------------------------ */
#define DEVICE_PATH "/dev/V853_MULTI_GPIO"

static const char *gpio_labels[] = {
    "VALVE1",  /* 水阀1  */
    "VALVE2",  /* 水阀2  */
    "VALVE3",  /* 水阀3  */
    "MAGNET",  /* 电磁铁 */
    "STEPPER", /* 步进电机 */
    "CAMERA"   /* 监控设备 */
};

static int gpio_fd = -1;
static unsigned char gpio_state = 0x00;

void gpio_init(void)
{
    if (gpio_fd < 0) {
        gpio_fd = open(DEVICE_PATH, O_WRONLY);
        if (gpio_fd < 0) perror("open /dev/V853_MULTI_GPIO failed");
    }
}

void gpio_cleanup(void)
{
    if (gpio_fd >= 0) { close(gpio_fd); gpio_fd = -1; }
}

static void gpio_control(int id, bool enable)
{
    if (gpio_fd < 0) { printf("gpio_control: not initialized\n"); return; }
    if (id < 0 || id > 7) { printf("Invalid GPIO id: %d\n", id); return; }

    if (enable) gpio_state |=  (1 << id);
    else        gpio_state &= ~(1 << id);

    if (write(gpio_fd, &gpio_state, 0) != 0)
        perror("write gpio control failed");
    else
        printf("GPIO[%d] %s => state=0x%02X\n", id, enable ? "ON" : "OFF", gpio_state);
}

static void gpio_btn_event_cb(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    uint32_t  id  = (uint32_t)lv_event_get_user_data(e);
    bool on = lv_obj_has_state(btn, LV_STATE_CHECKED);
    gpio_control(id, on);
}

/* ------------------------------------------------------------------ */
/*  Camera canvas                                                       */
/* ------------------------------------------------------------------ */
static lv_obj_t   *g_cam_canvas  = NULL;
static lv_timer_t *g_cam_timer   = NULL;
static uint32_t    g_cam_buf[CAM_H * CAM_W];
static int         g_cam_last_seq = 0;
static bool        g_cam_active   = false;

static void cam_timer_cb(lv_timer_t *t)
{
    (void)t;
    if (!g_cam_active) return;
    if (cam_copy_frame(g_cam_buf, &g_cam_last_seq))
        lv_obj_invalidate(g_cam_canvas);
}

static void cam_canvas_create(void)
{
    g_cam_canvas = lv_canvas_create(lv_scr_act());
    lv_canvas_set_buffer(g_cam_canvas, g_cam_buf,
                         CAM_W, CAM_H, LV_COLOR_FORMAT_XRGB8888);
    lv_obj_set_pos(g_cam_canvas, 0, 0);
    lv_obj_add_flag(g_cam_canvas, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(g_cam_canvas, LV_OBJ_FLAG_CLICKABLE);

    /* 33 ms ≈ 30 fps; starts paused */
    g_cam_timer = lv_timer_create(cam_timer_cb, 33, NULL);
    lv_timer_pause(g_cam_timer);
}

/* ------------------------------------------------------------------ */
/*  camON / camOFF button callbacks                                     */
/*                                                                      */
/*  The camera thread is created once at startup and keeps the ISP     */
/*  open permanently.  ON/OFF only toggles frame delivery to avoid     */
/*  the V851S ISP crash that occurs on cap.release() + cap.open().     */
/* ------------------------------------------------------------------ */
static void start_btn_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (g_cam_active) return;
    if (!cam_is_open()) {
        fprintf(stderr, "[ui] 摄像头未就绪\n");
        return;
    }
    g_cam_last_seq = 0;
    g_cam_active   = true;
    cam_capture_start();
    lv_obj_remove_flag(g_cam_canvas, LV_OBJ_FLAG_HIDDEN);
    lv_timer_resume(g_cam_timer);
}

static void stop_btn_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (!g_cam_active) return;
    g_cam_active = false;
    cam_capture_stop();
    lv_timer_pause(g_cam_timer);
    lv_obj_add_flag(g_cam_canvas, LV_OBJ_FLAG_HIDDEN);
    memset(g_cam_buf, 0, sizeof(g_cam_buf));
}

/* ------------------------------------------------------------------ */
/*  UI panels (right side: x=500, logical 800×480)                     */
/* ------------------------------------------------------------------ */
static void lv_demo_gpio_button(void)
{
    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, 280, 200);
    lv_obj_set_pos(cont, 505, 10);
    lv_obj_set_layout(cont, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(cont, 8, 0);
    lv_obj_set_style_pad_column(cont, 8, 0);
    lv_obj_set_style_pad_row(cont, 8, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    for (int i = 0; i < 6; i++) {
        lv_obj_t *btn = lv_button_create(cont);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(btn, 120, 50);
        lv_obj_add_event_cb(btn, gpio_btn_event_cb, LV_EVENT_VALUE_CHANGED,
                            (void *)(uintptr_t)i);
        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, gpio_labels[i]);
        lv_obj_center(label);
        lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_GREEN),
                                  LV_PART_MAIN | LV_STATE_CHECKED);
    }
}

static void lv_demo_cam_button(void)
{
    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, 280, 80);
    lv_obj_set_pos(cont, 505, 225);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(cont, 10, 0);
    lv_obj_set_style_pad_column(cont, 10, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *btn_on = lv_button_create(cont);
    lv_obj_set_size(btn_on, 120, 50);
    lv_obj_add_event_cb(btn_on, start_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl1 = lv_label_create(btn_on);
    lv_label_set_text(lbl1, "camON");
    lv_obj_center(lbl1);

    lv_obj_t *btn_off = lv_button_create(cont);
    lv_obj_set_size(btn_off, 120, 50);
    lv_obj_add_event_cb(btn_off, stop_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl2 = lv_label_create(btn_off);
    lv_label_set_text(lbl2, "camOFF");
    lv_obj_center(lbl2);
}

/* ------------------------------------------------------------------ */
/*  Display / input init                                                */
/* ------------------------------------------------------------------ */
#if LV_USE_WAYLAND
#include "backends/interface.h"
#endif

uint16_t window_width;
uint16_t window_height;
bool fullscreen;
bool maximize;
void lv_linux_run_loop(void);

static const char *getenv_default(const char *name, const char *dflt)
{
    return getenv(name) ?: dflt;
}

#if LV_USE_EVDEV
static void lv_linux_init_input_pointer(lv_display_t *disp)
{
    const char *input_device =
        getenv_default("LV_LINUX_EVDEV_POINTER_DEVICE", "/dev/input/event0");
    if (!input_device) {
        fprintf(stderr, "please set LV_LINUX_EVDEV_POINTER_DEVICE\n");
        exit(1);
    }
    lv_indev_t *touch = lv_evdev_create(LV_INDEV_TYPE_POINTER, input_device);
    lv_indev_set_display(touch, disp);
    LV_IMAGE_DECLARE(mouse_cursor_icon);
    lv_obj_t *cursor_obj = lv_image_create(lv_screen_active());
    lv_image_set_src(cursor_obj, &mouse_cursor_icon);
    lv_indev_set_cursor(touch, cursor_obj);
}
#endif

#if LV_USE_LINUX_FBDEV
static void lv_linux_disp_init(void)
{
    const char *device = getenv_default("LV_LINUX_FBDEV_DEVICE", "/dev/fb0");
    lv_display_t *disp = lv_linux_fbdev_create();
    lv_display_set_rotation(NULL, LV_DISPLAY_ROTATION_90);
#if LV_USE_EVDEV
    lv_linux_init_input_pointer(disp);
#endif
    lv_linux_fbdev_set_file(disp, device);
}
#elif LV_USE_LINUX_DRM
static void lv_linux_disp_init(void)
{
    const char *device = getenv_default("LV_LINUX_DRM_CARD", "/dev/dri/card0");
    lv_display_t *disp = lv_linux_drm_create();
#if LV_USE_EVDEV
    lv_linux_init_input_pointer(disp);
#endif
    lv_linux_drm_set_file(disp, device, -1);
}
#elif LV_USE_SDL
static void lv_linux_disp_init(void)
{
    lv_sdl_window_create(window_width, window_height);
}
#elif LV_USE_WAYLAND
/* see backend/wayland.c */
#else
#error Unsupported configuration
#endif

#if LV_USE_WAYLAND == 0
void lv_linux_run_loop(void)
{
    uint32_t idle_time;
    while (1) {
        idle_time = lv_timer_handler();
        usleep(idle_time * 1000);
    }
}
#endif

static void configure_simulator(int argc, char **argv)
{
    int opt = 0;
    fullscreen = maximize = false;
    window_width  = atoi(getenv("LV_SIM_WINDOW_WIDTH")  ?: "480");
    window_height = atoi(getenv("LV_SIM_WINDOW_HEIGHT") ?: "800");

    while ((opt = getopt(argc, argv, "fmw:h:")) != -1) {
        switch (opt) {
        case 'f':
            fullscreen = true;
            if (LV_USE_WAYLAND == 0) {
                fprintf(stderr, "SDL doesn't support fullscreen on start\n");
                exit(1);
            }
            break;
        case 'm':
            maximize = true;
            if (LV_USE_WAYLAND == 0) {
                fprintf(stderr, "SDL doesn't support maximized on start\n");
                exit(1);
            }
            break;
        case 'w': window_width  = atoi(optarg); break;
        case 'h': window_height = atoi(optarg); break;
        case ':': fprintf(stderr, "Option -%c requires argument.\n", optopt); exit(1);
        case '?': fprintf(stderr, "Unknown option -%c.\n", optopt); exit(1);
        }
    }
}

/* ------------------------------------------------------------------ */
/*  main                                                                */
/* ------------------------------------------------------------------ */
int main(int argc, char **argv)
{
    configure_simulator(argc, argv);

    lv_init();
    gpio_init();
    lv_linux_disp_init();

    /* Start camera thread once – opens ISP and stays open permanently */
    cam_thread_init();

    /* Left side: camera canvas (500x480 at origin) */
    cam_canvas_create();

    /* Right side: GPIO buttons and cam control buttons */
    lv_demo_gpio_button();
    lv_demo_cam_button();

    lv_linux_run_loop();

    return 0;
}
