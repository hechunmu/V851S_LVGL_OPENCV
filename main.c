#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "lv_100ask_lesson_demos/lv_100ask_lesson_demos.h"
#include "lv_port_indev.h"
#include "cam_thread.h"
#include "stream_thread.h"
#include "app_gpio.h"
#include "app_uart.h"
#include "wifi_manager.h"
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <signal.h>
#include <sys/types.h>

/* ================================================================== */
/*  Design tokens                                                      */
/* ================================================================== */
#define C_BG       lv_color_hex(0x070B10)
#define C_SURFACE  lv_color_hex(0x0E1726)
#define C_CARD     lv_color_hex(0x111C2A)
#define C_ACCENT   lv_color_hex(0x00D4FF)
#define C_GREEN    lv_color_hex(0x00FF94)
#define C_RED      lv_color_hex(0xFF2D55)
#define C_AMBER    lv_color_hex(0xFFAA00)
#define C_TEXT     lv_color_hex(0xDCEFFF)
#define C_DIM      lv_color_hex(0x4A6280)
#define C_BORDER   lv_color_hex(0x1C3450)

/* ================================================================== */
/*  App / GPIO / UART state                                            */
/* ================================================================== */
static volatile sig_atomic_t g_app_running = 1;
static const char *getenv_default(const char *name, const char *dflt);

static const uint8_t uart_cmd_valve1_on[]  = { 0x55, 0x43, 0x11, 0x07 };
static const uint8_t uart_cmd_valve1_off[] = { 0x55, 0x43, 0x01, 0x17 };

static void app_handle_gpio_button(uint32_t id, bool enable, void *user_data)
{
    (void)user_data;
    app_gpio_apply_output(id, enable, false);
    if (id == APP_GPIO_VALVE1 && enable)
        (void)app_uart_send_frame(uart_cmd_valve1_on, sizeof(uart_cmd_valve1_on));
}

static void app_handle_uart_frame(const uint8_t frame[4], void *user_data)
{
    (void)user_data;
    if (memcmp(frame, uart_cmd_valve1_off, sizeof(uart_cmd_valve1_off)) == 0)
        app_gpio_apply_output(APP_GPIO_VALVE1, false, true);
}

static void gpio_init(void)    { app_gpio_init(); }
static void gpio_cleanup(void) { app_gpio_cleanup(); }
static void uart_init(void)    { app_uart_init(&g_app_running, app_handle_uart_frame, NULL); }
static void uart_cleanup(void) { app_uart_cleanup(); }

/* ================================================================== */
/*  Camera state                                                       */
/* ================================================================== */
static lv_obj_t   *g_cam_canvas  = NULL;
static lv_timer_t *g_cam_timer   = NULL;
static uint32_t    g_cam_buf[CAM_H * CAM_W];
static int         g_cam_last_seq = 0;
static bool        g_cam_active   = false;

/* ================================================================== */
/*  Page references                                                    */
/* ================================================================== */
static lv_obj_t *g_page_home = NULL;
static lv_obj_t *g_page_cam  = NULL;
static lv_obj_t *g_page_ctrl = NULL;
static lv_obj_t *g_page_wifi = NULL;

/* Dynamic widget refs */
static lv_obj_t *g_clock_lbl   = NULL;
static lv_obj_t *g_rec_dot     = NULL;
static lv_obj_t *g_rec_btn     = NULL;
static lv_obj_t *g_rec_label   = NULL;
static lv_obj_t *g_wifi_list   = NULL;   /* scrollable list on wifi page */
static lv_obj_t *g_wifi_status = NULL;   /* status label on wifi page    */
static lv_timer_t *g_wifi_poll = NULL;   /* poll timer for scan/connect  */

/* ================================================================== */
/*  Page navigation helpers                                            */
/* ================================================================== */
static lv_obj_t *g_wifi_modal = NULL;   /* declared early for use in nav */

static void show_page(lv_obj_t *page)
{
    /* Close WiFi password modal if open before any navigation */
    if (g_wifi_modal) {
        lv_obj_del(g_wifi_modal);
        g_wifi_modal = NULL;
    }
    if (g_page_home) lv_obj_add_flag(g_page_home, LV_OBJ_FLAG_HIDDEN);
    if (g_page_cam)  lv_obj_add_flag(g_page_cam,  LV_OBJ_FLAG_HIDDEN);
    if (g_page_ctrl) lv_obj_add_flag(g_page_ctrl, LV_OBJ_FLAG_HIDDEN);
    if (g_page_wifi) lv_obj_add_flag(g_page_wifi, LV_OBJ_FLAG_HIDDEN);
    if (page)        lv_obj_remove_flag(page, LV_OBJ_FLAG_HIDDEN);
}

/* Stop camera-related activity — only called when LEAVING camera page */
static void cam_stop_all(void)
{
    if (cam_is_recording()) cam_record_stop();
    if (g_cam_active) {
        g_cam_active = false;
        cam_capture_stop();
        if (g_cam_timer) lv_timer_pause(g_cam_timer);
    }
    /* Reset record button visual state (safe: button persists in hidden page) */
    if (g_rec_btn)   lv_obj_set_style_bg_color(g_rec_btn, lv_color_hex(0x1A1A2E), 0);
    if (g_rec_label) lv_label_set_text(g_rec_label, LV_SYMBOL_PLAY " REC");
}

/* ================================================================== */
/*  Shared style helpers                                               */
/* ================================================================== */
static lv_obj_t *bare_cont(lv_obj_t *parent)
{
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_remove_flag(o, LV_OBJ_FLAG_SCROLLABLE);
    return o;
}

static void accent_line(lv_obj_t *parent, int y)
{
    lv_obj_t *l = bare_cont(parent);
    lv_obj_set_size(l, 800, 2);
    lv_obj_set_pos(l, 0, y);
    lv_obj_set_style_bg_color(l, C_ACCENT, 0);
    lv_obj_set_style_bg_opa(l, LV_OPA_40, 0);
}

/* Top bar shared by CTRL and WIFI pages */
static lv_obj_t *make_top_bar(lv_obj_t *page, const char *title, lv_event_cb_t back_cb)
{
    lv_obj_t *bar = bare_cont(page);
    lv_obj_set_size(bar, 800, 52);
    lv_obj_set_pos(bar, 0, 0);
    lv_obj_set_style_bg_color(bar, C_SURFACE, 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);

    /* Back button */
    lv_obj_t *back = lv_button_create(bar);
    lv_obj_remove_style_all(back);
    lv_obj_set_size(back, 80, 36);
    lv_obj_align(back, LV_ALIGN_LEFT_MID, 50, 0);
    lv_obj_set_style_bg_color(back, lv_color_hex(0x1A2030), 0);
    lv_obj_set_style_bg_opa(back, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(back, C_ACCENT, 0);
    lv_obj_set_style_border_width(back, 1, 0);
    lv_obj_set_style_radius(back, 18, 0);
    lv_obj_add_event_cb(back, back_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *bl = lv_label_create(back);
    lv_label_set_text(bl, LV_SYMBOL_LEFT " BACK");
    lv_obj_set_style_text_font(bl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(bl, C_ACCENT, 0);
    lv_obj_center(bl);

    /* Title */
    lv_obj_t *tl = lv_label_create(bar);
    lv_label_set_text(tl, title);
    lv_obj_set_style_text_font(tl, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(tl, C_TEXT, 0);
    lv_obj_align(tl, LV_ALIGN_CENTER, 0, 0);

    accent_line(page, 52);
    return bar;
}

/* ================================================================== */
/*  Timestamps                                                        */
/* ================================================================== */
static void make_ts_path(char *buf, size_t sz, const char *dir,
                         const char *pfx, const char *ext)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    snprintf(buf, sz, "%s/%s_%04d%02d%02d_%02d%02d%02d.%s",
             dir, pfx, t->tm_year+1900, t->tm_mon+1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec, ext);
}

/* ================================================================== */
/*  Camera timer (30 fps + blink indicator)                           */
/* ================================================================== */
static void cam_timer_cb(lv_timer_t *t)
{
    static int blink_cnt = 0;
    (void)t;
    if (!g_cam_active) return;
    if (cam_copy_frame(g_cam_buf, &g_cam_last_seq))
        lv_obj_invalidate(g_cam_canvas);

    if (g_rec_dot) {
        if (cam_is_recording()) {
            if (++blink_cnt >= 15) {
                blink_cnt = 0;
                if (lv_obj_has_flag(g_rec_dot, LV_OBJ_FLAG_HIDDEN))
                    lv_obj_remove_flag(g_rec_dot, LV_OBJ_FLAG_HIDDEN);
                else
                    lv_obj_add_flag(g_rec_dot, LV_OBJ_FLAG_HIDDEN);
            }
        } else {
            blink_cnt = 0;
            lv_obj_add_flag(g_rec_dot, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

/* ================================================================== */
/*  Nav callbacks                                                      */
/* ================================================================== */

/* Back from CAMERA page: stop camera then go home */
static void nav_home_from_cam_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    cam_stop_all();
    show_page(g_page_home);
}

/* Back from CONTROL / WIFI pages: just go home, never touch camera state */
static void nav_home_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    show_page(g_page_home);
}

static void nav_cam_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    show_page(g_page_cam);
    /* Start camera if not already active.
     * If ISP not ready yet cam_capture_start is harmless — write_frame
     * is only called when s_capture==1, and cam_timer_cb checks g_cam_active. */
    if (!g_cam_active) {
        g_cam_last_seq = 0;
        g_cam_active   = true;
        cam_capture_start();
        if (g_cam_timer) lv_timer_resume(g_cam_timer);
    }
}

static void nav_ctrl_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    show_page(g_page_ctrl);
}

static void nav_wifi_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    show_page(g_page_wifi);
    wifi_scan_async();
}

/* ================================================================== */
/*  Camera button callbacks                                            */
/* ================================================================== */
static void photo_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (!g_cam_active) return;
    char path[128];
    make_ts_path(path, sizeof(path), "/mnt/extsd", "photo", "jpg");
    cam_save_photo(path);
}

static void record_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (!g_cam_active) return;
    lv_obj_t *btn = lv_event_get_target(e);
    if (!cam_is_recording()) {
        char path[128];
        make_ts_path(path, sizeof(path), "/mnt/extsd", "video", "mjpeg");
        if (cam_record_start(path) == 0) {
            lv_obj_set_style_bg_color(btn, C_RED, 0);
            lv_obj_set_style_border_color(btn, C_RED, 0);
            if (g_rec_label) lv_label_set_text(g_rec_label, LV_SYMBOL_STOP " STOP");
        }
    } else {
        cam_record_stop();
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x1A1A2E), 0);
        lv_obj_set_style_border_color(btn, C_RED, 0);
        if (g_rec_label) lv_label_set_text(g_rec_label, LV_SYMBOL_PLAY " REC");
    }
}

/* ================================================================== */
/*  Clock timer                                                        */
/* ================================================================== */
static void clock_cb(lv_timer_t *t)
{
    (void)t;
    if (!g_clock_lbl) return;
    char buf[24];
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    strftime(buf, sizeof(buf), "%H:%M:%S", tm);
    lv_label_set_text(g_clock_lbl, buf);
}

/* ================================================================== */
/*  HOME PAGE                                                          */
/* ================================================================== */
typedef struct {
    const char   *icon;
    const char   *title;
    const char   *sub;
    uint32_t      color_hex;
    lv_event_cb_t cb;
} card_cfg_t;

static void build_home_card(lv_obj_t *parent, const card_cfg_t *cfg)
{
    lv_color_t col = lv_color_hex(cfg->color_hex);

    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_remove_style_all(card);
    lv_obj_remove_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(card, 180, 340);
    lv_obj_add_flag(card, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(card, C_CARD, 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(card, C_BORDER, 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_radius(card, 16, 0);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x1A2B3E), LV_STATE_PRESSED);
    lv_obj_set_style_border_color(card, col, LV_STATE_PRESSED);
    lv_obj_set_style_border_width(card, 2, LV_STATE_PRESSED);
    lv_obj_set_layout(card, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(card, 18, 0);
    lv_obj_set_style_pad_row(card, 16, 0);

    /* Icon circle */
    lv_obj_t *circle = lv_obj_create(card);
    lv_obj_remove_style_all(circle);
    lv_obj_set_size(circle, 88, 88);
    lv_obj_set_style_radius(circle, 44, 0);
    lv_obj_set_style_bg_color(circle, col, 0);
    lv_obj_set_style_bg_opa(circle, LV_OPA_20, 0);
    lv_obj_set_style_border_color(circle, col, 0);
    lv_obj_set_style_border_width(circle, 2, 0);
    lv_obj_set_style_border_opa(circle, LV_OPA_60, 0);
    lv_obj_remove_flag(circle, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *icon = lv_label_create(circle);
    lv_label_set_text(icon, cfg->icon);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_38, 0);
    lv_obj_set_style_text_color(icon, col, 0);
    lv_obj_center(icon);

    /* Separator */
    lv_obj_t *sep = bare_cont(card);
    lv_obj_set_size(sep, 50, 1);
    lv_obj_set_style_bg_color(sep, col, 0);
    lv_obj_set_style_bg_opa(sep, LV_OPA_40, 0);

    /* Title */
    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, cfg->title);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(title, C_TEXT, 0);

    /* Subtitle */
    lv_obj_t *sub = lv_label_create(card);
    lv_label_set_text(sub, cfg->sub);
    lv_obj_set_style_text_font(sub, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(sub, C_DIM, 0);

    if (cfg->cb) lv_obj_add_event_cb(card, cfg->cb, LV_EVENT_CLICKED, NULL);
}

static void build_home_page(void)
{
    lv_obj_t *p = g_page_home;
    lv_obj_set_style_bg_color(p, C_BG, 0);
    lv_obj_set_style_bg_opa(p, LV_OPA_COVER, 0);

    /* Top bar */
    lv_obj_t *bar = bare_cont(p);
    lv_obj_set_size(bar, 800, 48);
    lv_obj_set_pos(bar, 0, 0);
    lv_obj_set_style_bg_color(bar, C_SURFACE, 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);

    lv_obj_t *title = lv_label_create(bar);
    lv_label_set_text(title, LV_SYMBOL_SETTINGS "  CONTROL SYSTEM");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(title, C_ACCENT, 0);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 16, 0);

    g_clock_lbl = lv_label_create(bar);
    lv_label_set_text(g_clock_lbl, "00:00:00");
    lv_obj_set_style_text_font(g_clock_lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(g_clock_lbl, C_GREEN, 0);
    lv_obj_align(g_clock_lbl, LV_ALIGN_RIGHT_MID, -16, 0);

    accent_line(p, 48);

    /* Cards area */
    lv_obj_t *cards = bare_cont(p);
    lv_obj_set_size(cards, 800, 426);
    lv_obj_set_pos(cards, 0, 52);
    lv_obj_set_layout(cards, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(cards, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cards, LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    static const card_cfg_t cfgs[] = {
        { LV_SYMBOL_IMAGE,    "CAMERA",  "Live preview",    0x00D4FF, nav_cam_cb  },
        { LV_SYMBOL_SETTINGS, "CONTROL", "GPIO outputs",    0x00FF94, nav_ctrl_cb },
        { LV_SYMBOL_WIFI,     "WiFi",    "Network manager", 0xFFAA00, nav_wifi_cb },
        { LV_SYMBOL_LIST,     "SYSTEM",  "Info & status",   0xBB66FF, NULL        },
    };
    for (int i = 0; i < 4; i++) build_home_card(cards, &cfgs[i]);

    lv_timer_create(clock_cb, 1000, NULL);
    clock_cb(NULL);
}

/* ================================================================== */
/*  CAMERA PAGE                                                        */
/* ================================================================== */
static lv_obj_t *cam_overlay(lv_obj_t *parent, int y, int h)
{
    lv_obj_t *s = bare_cont(parent);
    lv_obj_set_size(s, 800, h);
    lv_obj_set_pos(s, 0, y);
    lv_obj_set_style_bg_color(s, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(s, LV_OPA_70, 0);
    return s;
}

static void build_cam_page(void)
{
    lv_obj_t *p = g_page_cam;
    lv_obj_set_style_bg_color(p, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(p, LV_OPA_COVER, 0);

    /* Camera canvas (lowest Z-order) */
    g_cam_canvas = lv_canvas_create(p);
    lv_canvas_set_buffer(g_cam_canvas, g_cam_buf,
                         CAM_W, CAM_H, LV_COLOR_FORMAT_XRGB8888);
    lv_obj_set_pos(g_cam_canvas, 0, 0);
    lv_obj_clear_flag(g_cam_canvas, LV_OBJ_FLAG_CLICKABLE);
    g_cam_timer = lv_timer_create(cam_timer_cb, 33, NULL);
    lv_timer_pause(g_cam_timer);

    /* Top overlay */
    lv_obj_t *top = cam_overlay(p, 0, 68);

    lv_obj_t *back = lv_button_create(top);
    lv_obj_remove_style_all(back);
    lv_obj_set_size(back, 80, 36);
    lv_obj_align(back, LV_ALIGN_LEFT_MID, 50, 0);
    lv_obj_set_style_bg_color(back, lv_color_hex(0x1A2030), 0);
    lv_obj_set_style_bg_opa(back, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(back, C_ACCENT, 0);
    lv_obj_set_style_border_width(back, 1, 0);
    lv_obj_set_style_radius(back, 18, 0);
    /* Use dedicated callback — only camera page stops the camera on back */
    lv_obj_add_event_cb(back, nav_home_from_cam_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *bl = lv_label_create(back);
    lv_label_set_text(bl, LV_SYMBOL_LEFT " BACK");
    lv_obj_set_style_text_font(bl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(bl, C_ACCENT, 0);
    lv_obj_center(bl);

    lv_obj_t *live = lv_label_create(top);
    lv_label_set_text(live, LV_SYMBOL_PLAY " LIVE");
    lv_obj_set_style_text_font(live, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(live, C_RED, 0);
    lv_obj_align(live, LV_ALIGN_CENTER, 0, 0);

    g_rec_dot = bare_cont(top);
    lv_obj_set_size(g_rec_dot, 12, 12);
    lv_obj_set_style_radius(g_rec_dot, 6, 0);
    lv_obj_set_style_bg_color(g_rec_dot, C_RED, 0);
    lv_obj_set_style_bg_opa(g_rec_dot, LV_OPA_COVER, 0);
    lv_obj_align(g_rec_dot, LV_ALIGN_RIGHT_MID, -12, 0);
    lv_obj_add_flag(g_rec_dot, LV_OBJ_FLAG_HIDDEN);

    /* Bottom overlay */
    lv_obj_t *bot = cam_overlay(p, 370, 110);

    /* Photo button — white circle (iOS shutter) */
    lv_obj_t *btn_ph = lv_button_create(bot);
    lv_obj_remove_style_all(btn_ph);
    lv_obj_set_size(btn_ph, 76, 76);
    lv_obj_set_style_radius(btn_ph, 38, 0);
    lv_obj_set_style_bg_color(btn_ph, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(btn_ph, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(btn_ph, lv_color_hex(0x666666), 0);
    lv_obj_set_style_border_width(btn_ph, 3, 0);
    lv_obj_set_style_bg_color(btn_ph, lv_color_hex(0xCCCCCC), LV_STATE_PRESSED);
    lv_obj_align(btn_ph, LV_ALIGN_CENTER, -120, 0);
    lv_obj_add_event_cb(btn_ph, photo_cb, LV_EVENT_CLICKED, NULL);

    /* inner ring */
    lv_obj_t *inner = bare_cont(btn_ph);
    lv_obj_set_size(inner, 60, 60);
    lv_obj_set_style_radius(inner, 30, 0);
    lv_obj_set_style_bg_color(inner, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(inner, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(inner, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_border_width(inner, 1, 0);
    lv_obj_center(inner);

    /* Photo caption */
    lv_obj_t *cap = lv_label_create(bot);
    lv_label_set_text(cap, "PHOTO");
    lv_obj_set_style_text_font(cap, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(cap, C_DIM, 0);
    lv_obj_align_to(cap, btn_ph, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);

    /* Record button */
    g_rec_btn = lv_button_create(bot);
    lv_obj_remove_style_all(g_rec_btn);
    lv_obj_set_size(g_rec_btn, 140, 52);
    lv_obj_set_style_radius(g_rec_btn, 26, 0);
    lv_obj_set_style_bg_color(g_rec_btn, lv_color_hex(0x1A1A2E), 0);
    lv_obj_set_style_bg_opa(g_rec_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(g_rec_btn, C_RED, 0);
    lv_obj_set_style_border_width(g_rec_btn, 2, 0);
    lv_obj_set_style_bg_color(g_rec_btn, C_RED, LV_STATE_PRESSED);
    lv_obj_align(g_rec_btn, LV_ALIGN_CENTER, 110, 0);
    lv_obj_add_event_cb(g_rec_btn, record_cb, LV_EVENT_CLICKED, NULL);

    g_rec_label = lv_label_create(g_rec_btn);
    lv_label_set_text(g_rec_label, LV_SYMBOL_PLAY " REC");
    lv_obj_set_style_text_font(g_rec_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(g_rec_label, C_RED, 0);
    lv_obj_center(g_rec_label);
}

/* ================================================================== */
/*  CONTROL PAGE                                                       */
/* ================================================================== */
static void build_ctrl_page(void)
{
    lv_obj_t *p = g_page_ctrl;
    lv_obj_set_style_bg_color(p, C_BG, 0);
    lv_obj_set_style_bg_opa(p, LV_OPA_COVER, 0);

    make_top_bar(p, LV_SYMBOL_SETTINGS "  DEVICE CONTROL", nav_home_cb);

    lv_obj_t *area = bare_cont(p);
    lv_obj_set_size(area, 800, 424);
    lv_obj_set_pos(area, 0, 54);
    lv_obj_set_style_bg_color(area, C_BG, 0);
    lv_obj_set_style_bg_opa(area, LV_OPA_COVER, 0);

    app_gpio_create_button_panel(area, app_handle_gpio_button, NULL);
}

/* ================================================================== */
/*  WIFI PAGE — signal bars widget                                     */
/* ================================================================== */
static void draw_signal_bars(lv_obj_t *parent, int bars)
{
    /* 4 bars of increasing height, bottom-aligned, in a 34x24 area */
    static const int H[] = { 8, 13, 18, 24 };
    static const int BW = 6, GAP = 3;
    for (int i = 0; i < 4; i++) {
        lv_obj_t *b = bare_cont(parent);
        lv_obj_set_size(b, BW, H[i]);
        lv_obj_set_pos(b, i * (BW + GAP), 24 - H[i]);
        lv_obj_set_style_radius(b, 1, 0);
        bool active = (i < bars);   /* bars=1→only bar0, bars=4→all */
        lv_obj_set_style_bg_color(b, active ? C_ACCENT : C_DIM, 0);
        lv_obj_set_style_bg_opa(b, active ? LV_OPA_COVER : LV_OPA_40, 0);
    }
}

/* ------------------------------------------------------------------ */
/*  WiFi password modal                                               */
/* ------------------------------------------------------------------ */
typedef struct {
    char ssid[WIFI_SSID_MAX];
    bool secured;
} wifi_connect_ctx_t;

/* g_wifi_modal declared at top of file (near show_page) */

static void wifi_modal_close(lv_event_t *e)
{
    (void)e;
    if (g_wifi_modal) {
        lv_obj_del(g_wifi_modal);
        g_wifi_modal = NULL;
    }
}

static void wifi_do_connect_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    lv_obj_t *ta = (lv_obj_t *)lv_event_get_user_data(e);
    wifi_connect_ctx_t *ctx = (wifi_connect_ctx_t *)lv_obj_get_user_data(ta);
    if (!ctx) return;

    const char *pw = lv_textarea_get_text(ta);
    wifi_connect_async(ctx->ssid, pw);

    if (g_wifi_status)
        lv_label_set_text(g_wifi_status, "Connecting...");

    if (g_wifi_modal) { lv_obj_del(g_wifi_modal); g_wifi_modal = NULL; }
}

static void wifi_kb_ready_cb(lv_event_t *e)
{
    /* Enter key on keyboard → connect */
    if (lv_event_get_code(e) != LV_EVENT_READY) return;
    lv_obj_t *kb = lv_event_get_target(e);
    lv_obj_t *ta = lv_keyboard_get_textarea(kb);
    if (!ta) return;
    wifi_connect_ctx_t *ctx = (wifi_connect_ctx_t *)lv_obj_get_user_data(ta);
    if (!ctx) return;

    const char *pw = lv_textarea_get_text(ta);
    wifi_connect_async(ctx->ssid, pw);
    if (g_wifi_status) lv_label_set_text(g_wifi_status, "Connecting...");
    if (g_wifi_modal)  { lv_obj_del(g_wifi_modal); g_wifi_modal = NULL; }
}

static void wifi_kb_cancel_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CANCEL) return;
    if (g_wifi_modal) { lv_obj_del(g_wifi_modal); g_wifi_modal = NULL; }
}

static void wifi_show_password_modal(const char *ssid, bool secured)
{
    if (g_wifi_modal) { lv_obj_del(g_wifi_modal); g_wifi_modal = NULL; }

    /* Modal root: full-screen semi-transparent overlay */
    g_wifi_modal = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(g_wifi_modal);
    lv_obj_set_size(g_wifi_modal, 800, 480);
    lv_obj_set_pos(g_wifi_modal, 0, 0);
    lv_obj_set_style_bg_color(g_wifi_modal, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(g_wifi_modal, LV_OPA_80, 0);
    lv_obj_remove_flag(g_wifi_modal, LV_OBJ_FLAG_SCROLLABLE);

    /* ------ Dialog panel (top half, 800×230) ------ */
    lv_obj_t *dialog = lv_obj_create(g_wifi_modal);
    lv_obj_remove_style_all(dialog);
    lv_obj_set_size(dialog, 700, 220);
    lv_obj_align(dialog, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_bg_color(dialog, lv_color_hex(0x0E1726), 0);
    lv_obj_set_style_bg_opa(dialog, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(dialog, C_ACCENT, 0);
    lv_obj_set_style_border_width(dialog, 1, 0);
    lv_obj_set_style_radius(dialog, 14, 0);
    lv_obj_set_style_pad_all(dialog, 20, 0);
    lv_obj_remove_flag(dialog, LV_OBJ_FLAG_SCROLLABLE);

    /* Title */
    char title_buf[80];
    snprintf(title_buf, sizeof(title_buf),
             LV_SYMBOL_WIFI "  Connect to: %s", ssid);
    lv_obj_t *title = lv_label_create(dialog);
    lv_label_set_text(title, title_buf);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(title, C_TEXT, 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 0);

    /* Password label */
    lv_obj_t *pw_lbl = lv_label_create(dialog);
    lv_label_set_text(pw_lbl, secured ? "Password:" : "(Open network — no password required)");
    lv_obj_set_style_text_font(pw_lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(pw_lbl, C_DIM, 0);
    lv_obj_align(pw_lbl, LV_ALIGN_TOP_LEFT, 0, 36);

    /* Password textarea */
    lv_obj_t *ta = lv_textarea_create(dialog);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_password_mode(ta, true);
    lv_textarea_set_placeholder_text(ta, secured ? "Enter password..." : "(no password)");
    lv_obj_set_size(ta, 620, 44);
    lv_obj_align(ta, LV_ALIGN_TOP_LEFT, 0, 60);
    lv_obj_set_style_bg_color(ta, lv_color_hex(0x0A1420), 0);
    lv_obj_set_style_bg_opa(ta, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(ta, C_BORDER, 0);
    lv_obj_set_style_border_width(ta, 1, 0);
    lv_obj_set_style_radius(ta, 8, 0);
    lv_obj_set_style_text_color(ta, C_TEXT, 0);
    lv_obj_set_style_text_font(ta, &lv_font_montserrat_16, 0);
    if (!secured) lv_obj_add_state(ta, LV_STATE_DISABLED);

    /* Allocate ctx and attach to textarea user_data */
    static wifi_connect_ctx_t ctx;
    strncpy(ctx.ssid, ssid, WIFI_SSID_MAX - 1);
    ctx.secured = secured;
    lv_obj_set_user_data(ta, &ctx);

    /* Cancel button */
    lv_obj_t *btn_cancel = lv_button_create(dialog);
    lv_obj_remove_style_all(btn_cancel);
    lv_obj_set_size(btn_cancel, 130, 40);
    lv_obj_align(btn_cancel, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0x1A2030), 0);
    lv_obj_set_style_bg_opa(btn_cancel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(btn_cancel, C_DIM, 0);
    lv_obj_set_style_border_width(btn_cancel, 1, 0);
    lv_obj_set_style_radius(btn_cancel, 10, 0);
    lv_obj_add_event_cb(btn_cancel, wifi_modal_close, LV_EVENT_CLICKED, NULL);
    lv_obj_t *cl = lv_label_create(btn_cancel);
    lv_label_set_text(cl, LV_SYMBOL_CLOSE "  Cancel");
    lv_obj_set_style_text_color(cl, C_DIM, 0);
    lv_obj_set_style_text_font(cl, &lv_font_montserrat_14, 0);
    lv_obj_center(cl);

    /* Connect button */
    lv_obj_t *btn_conn = lv_button_create(dialog);
    lv_obj_remove_style_all(btn_conn);
    lv_obj_set_size(btn_conn, 130, 40);
    lv_obj_align(btn_conn, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_style_bg_color(btn_conn, C_ACCENT, 0);
    lv_obj_set_style_bg_opa(btn_conn, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(btn_conn, 10, 0);
    lv_obj_set_style_bg_color(btn_conn, lv_color_hex(0x0099BB), LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn_conn, wifi_do_connect_cb, LV_EVENT_CLICKED, ta);
    lv_obj_t *col = lv_label_create(btn_conn);
    lv_label_set_text(col, LV_SYMBOL_OK "  Connect");
    lv_obj_set_style_text_color(col, lv_color_black(), 0);
    lv_obj_set_style_text_font(col, &lv_font_montserrat_14, 0);
    lv_obj_center(col);

    /* ------ LVGL keyboard (bottom half) ------ */
    lv_obj_t *kb = lv_keyboard_create(g_wifi_modal);
    lv_obj_set_size(kb, 800, 238);
    lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_keyboard_set_textarea(kb, ta);
    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_LOWER);
    lv_obj_set_style_bg_color(kb, lv_color_hex(0x0E1726), 0);
    lv_obj_set_style_bg_opa(kb, LV_OPA_COVER, 0);
    /* Key buttons */
    lv_obj_set_style_bg_color(kb, lv_color_hex(0x1A2A3C), LV_PART_ITEMS);
    lv_obj_set_style_border_color(kb, C_BORDER, LV_PART_ITEMS);
    lv_obj_set_style_border_width(kb, 1, LV_PART_ITEMS);
    lv_obj_set_style_text_color(kb, C_TEXT, LV_PART_ITEMS);
    lv_obj_set_style_radius(kb, 6, LV_PART_ITEMS);
    lv_obj_add_event_cb(kb, wifi_kb_ready_cb,  LV_EVENT_READY,  NULL);
    lv_obj_add_event_cb(kb, wifi_kb_cancel_cb, LV_EVENT_CANCEL, NULL);
}

/* ------------------------------------------------------------------ */
/*  WiFi list item click                                               */
/* ------------------------------------------------------------------ */
typedef struct {
    char ssid[WIFI_SSID_MAX];
    bool secured;
} wifi_item_data_t;

static void wifi_item_click_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    wifi_item_data_t *d = (wifi_item_data_t *)lv_event_get_user_data(e);
    if (!d) return;
    wifi_show_password_modal(d->ssid, d->secured);
}

/* ------------------------------------------------------------------ */
/*  Rebuild WiFi list from scan results                               */
/* ------------------------------------------------------------------ */
#define MAX_WIFI_ITEMS 20
static wifi_item_data_t g_wifi_item_data[MAX_WIFI_ITEMS];

static void rebuild_wifi_list(wifi_ap_t *aps, int count)
{
    if (!g_wifi_list) return;
    lv_obj_clean(g_wifi_list);   /* remove all children */

    if (count == 0) {
        lv_obj_t *empty = lv_label_create(g_wifi_list);
        lv_label_set_text(empty, "No networks found");
        lv_obj_set_style_text_color(empty, C_DIM, 0);
        lv_obj_set_style_text_font(empty, &lv_font_montserrat_16, 0);
        lv_obj_center(empty);
        return;
    }

    int n = count < MAX_WIFI_ITEMS ? count : MAX_WIFI_ITEMS;
    for (int i = 0; i < n; i++) {
        wifi_ap_t *ap = &aps[i];

        /* Store data for callback */
        strncpy(g_wifi_item_data[i].ssid, ap->ssid, WIFI_SSID_MAX - 1);
        g_wifi_item_data[i].secured = ap->secured;

        /* Row container */
        lv_obj_t *row = lv_obj_create(g_wifi_list);
        lv_obj_remove_style_all(row);
        lv_obj_set_size(row, 760, 66);
        lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(row, C_CARD, 0);
        lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(row, C_BORDER, 0);
        lv_obj_set_style_border_width(row, 1, 0);
        lv_obj_set_style_radius(row, 10, 0);
        lv_obj_set_style_bg_color(row, lv_color_hex(0x1A2B3E), LV_STATE_PRESSED);
        lv_obj_set_style_border_color(row, C_ACCENT, LV_STATE_PRESSED);
        lv_obj_add_event_cb(row, wifi_item_click_cb, LV_EVENT_CLICKED,
                            &g_wifi_item_data[i]);

        /* Connected highlight */
        if (ap->connected) {
            lv_obj_set_style_border_color(row, C_GREEN, 0);
            lv_obj_set_style_border_width(row, 2, 0);
        }

        /* -- Signal bars (left, 34x24 in a 50px wide area) -- */
        lv_obj_t *sig_cont = bare_cont(row);
        lv_obj_set_size(sig_cont, 34, 24);
        lv_obj_set_pos(sig_cont, 14, 21);
        draw_signal_bars(sig_cont, ap->bars);

        /* -- SSID label -- */
        lv_obj_t *ssid_lbl = lv_label_create(row);
        lv_label_set_text(ssid_lbl, ap->ssid);
        lv_obj_set_style_text_font(ssid_lbl, &lv_font_montserrat_18, 0);
        lv_obj_set_style_text_color(ssid_lbl,
            ap->connected ? C_GREEN : C_TEXT, 0);
        lv_obj_align(ssid_lbl, LV_ALIGN_LEFT_MID, 60, -8);
        lv_label_set_long_mode(ssid_lbl, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(ssid_lbl, 560);

        /* -- Security sub-label -- */
        lv_obj_t *sec_lbl = lv_label_create(row);
        lv_label_set_text(sec_lbl, ap->secured ? "WPA/WPA2" : "Open");
        lv_obj_set_style_text_font(sec_lbl, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(sec_lbl, C_DIM, 0);
        lv_obj_align(sec_lbl, LV_ALIGN_LEFT_MID, 60, 14);

        /* -- Lock icon (right) -- */
        if (ap->secured) {
            lv_obj_t *lock = lv_label_create(row);
            lv_label_set_text(lock, LV_SYMBOL_EYE_CLOSE);
            lv_obj_set_style_text_font(lock, &lv_font_montserrat_18, 0);
            lv_obj_set_style_text_color(lock, C_DIM, 0);
            lv_obj_align(lock, LV_ALIGN_RIGHT_MID, -44, 0);
        }

        /* -- Connected dot (far right) -- */
        if (ap->connected) {
            lv_obj_t *dot = bare_cont(row);
            lv_obj_set_size(dot, 10, 10);
            lv_obj_set_style_radius(dot, 5, 0);
            lv_obj_set_style_bg_color(dot, C_GREEN, 0);
            lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
            lv_obj_align(dot, LV_ALIGN_RIGHT_MID, -14, 0);
        }
    }
}

/* ------------------------------------------------------------------ */
/*  WiFi poll timer — updates UI when scan/connect completes           */
/* ------------------------------------------------------------------ */
static void wifi_poll_cb(lv_timer_t *t)
{
    (void)t;
    wifi_state_t st = wifi_get_state();

    if (g_wifi_status) lv_label_set_text(g_wifi_status, wifi_get_state_str());

    if (st == WIFI_STATE_SCAN_DONE) {
        wifi_ap_t aps[WIFI_MAX_AP];
        int n = wifi_get_results(aps, WIFI_MAX_AP);
        rebuild_wifi_list(aps, n);
        char sbuf[48];
        snprintf(sbuf, sizeof(sbuf), "%d network(s) found", n);
        if (g_wifi_status) lv_label_set_text(g_wifi_status, sbuf);
    } else if (st == WIFI_STATE_CONNECTED) {
        char ssid[WIFI_SSID_MAX] = "";
        wifi_get_connected_ssid(ssid, sizeof(ssid));
        char sbuf[80];
        snprintf(sbuf, sizeof(sbuf), LV_SYMBOL_OK "  Connected: %s", ssid);
        if (g_wifi_status) lv_label_set_text(g_wifi_status, sbuf);
        /* Refresh list to show connected marker */
        wifi_scan_async();
    } else if (st == WIFI_STATE_FAILED) {
        if (g_wifi_status) lv_label_set_text(g_wifi_status, "Connection failed");
    }
}

/* ------------------------------------------------------------------ */
/*  Scan button                                                        */
/* ------------------------------------------------------------------ */
static void wifi_scan_btn_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    wifi_scan_async();
    if (g_wifi_status) lv_label_set_text(g_wifi_status, "Scanning...");
    if (g_wifi_list) lv_obj_clean(g_wifi_list);
}

/* ------------------------------------------------------------------ */
/*  Build WiFi page                                                    */
/* ------------------------------------------------------------------ */
static void build_wifi_page(void)
{
    lv_obj_t *p = g_page_wifi;
    lv_obj_set_style_bg_color(p, C_BG, 0);
    lv_obj_set_style_bg_opa(p, LV_OPA_COVER, 0);

    /* Top bar (with extra scan button on right) */
    lv_obj_t *bar = bare_cont(p);
    lv_obj_set_size(bar, 800, 52);
    lv_obj_set_pos(bar, 0, 0);
    lv_obj_set_style_bg_color(bar, C_SURFACE, 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);

    /* Back button */
    lv_obj_t *back = lv_button_create(bar);
    lv_obj_remove_style_all(back);
    lv_obj_set_size(back, 80, 36);
    lv_obj_align(back, LV_ALIGN_LEFT_MID, 50, 0);
    lv_obj_set_style_bg_color(back, lv_color_hex(0x1A2030), 0);
    lv_obj_set_style_bg_opa(back, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(back, C_ACCENT, 0);
    lv_obj_set_style_border_width(back, 1, 0);
    lv_obj_set_style_radius(back, 18, 0);
    lv_obj_add_event_cb(back, nav_home_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *bl = lv_label_create(back);
    lv_label_set_text(bl, LV_SYMBOL_LEFT " BACK");
    lv_obj_set_style_text_font(bl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(bl, C_ACCENT, 0);
    lv_obj_center(bl);

    /* Title */
    lv_obj_t *title = lv_label_create(bar);
    lv_label_set_text(title, LV_SYMBOL_WIFI "  WiFi Networks");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(title, C_TEXT, 0);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);

    /* Scan button (top-right) */
    lv_obj_t *scan_btn = lv_button_create(bar);
    lv_obj_remove_style_all(scan_btn);
    lv_obj_set_size(scan_btn, 88, 36);
    lv_obj_align(scan_btn, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_set_style_bg_color(scan_btn, C_AMBER, 0);
    lv_obj_set_style_bg_opa(scan_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(scan_btn, 18, 0);
    lv_obj_set_style_bg_color(scan_btn, lv_color_hex(0xCC8800), LV_STATE_PRESSED);
    lv_obj_add_event_cb(scan_btn, wifi_scan_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *sl = lv_label_create(scan_btn);
    lv_label_set_text(sl, LV_SYMBOL_REFRESH " Scan");
    lv_obj_set_style_text_font(sl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(sl, lv_color_black(), 0);
    lv_obj_center(sl);

    accent_line(p, 52);

    /* Status bar */
    lv_obj_t *status_bar = bare_cont(p);
    lv_obj_set_size(status_bar, 800, 30);
    lv_obj_set_pos(status_bar, 0, 54);
    lv_obj_set_style_bg_color(status_bar, lv_color_hex(0x0A1020), 0);
    lv_obj_set_style_bg_opa(status_bar, LV_OPA_COVER, 0);

    g_wifi_status = lv_label_create(status_bar);
    lv_label_set_text(g_wifi_status, "Press Scan to search for networks");
    lv_obj_set_style_text_font(g_wifi_status, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(g_wifi_status, C_DIM, 0);
    lv_obj_align(g_wifi_status, LV_ALIGN_LEFT_MID, 16, 0);

    /* Scrollable WiFi list */
    g_wifi_list = lv_obj_create(p);
    lv_obj_remove_style_all(g_wifi_list);
    lv_obj_set_size(g_wifi_list, 800, 394);
    lv_obj_set_pos(g_wifi_list, 0, 86);
    lv_obj_set_scroll_dir(g_wifi_list, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(g_wifi_list, LV_SCROLLBAR_MODE_ACTIVE);
    lv_obj_set_layout(g_wifi_list, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(g_wifi_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(g_wifi_list, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_color(g_wifi_list, C_BG, 0);
    lv_obj_set_style_bg_opa(g_wifi_list, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(g_wifi_list, 12, 0);
    lv_obj_set_style_pad_row(g_wifi_list, 8, 0);

    /* Style scrollbar */
    lv_obj_set_style_bg_color(g_wifi_list, C_ACCENT, LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_opa(g_wifi_list, LV_OPA_40, LV_PART_SCROLLBAR);

    /* Poll timer: check WiFi state every 500 ms */
    g_wifi_poll = lv_timer_create(wifi_poll_cb, 500, NULL);
}

/* ================================================================== */
/*  Build all pages                                                    */
/* ================================================================== */
static void build_ui(void)
{
    lv_obj_set_style_bg_color(lv_scr_act(), C_BG, 0);
    lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0);

    /* Create full-screen page containers */
    g_page_home = bare_cont(lv_scr_act());
    lv_obj_set_size(g_page_home, 800, 480); lv_obj_set_pos(g_page_home, 0, 0);

    g_page_cam = bare_cont(lv_scr_act());
    lv_obj_set_size(g_page_cam, 800, 480);  lv_obj_set_pos(g_page_cam, 0, 0);

    g_page_ctrl = bare_cont(lv_scr_act());
    lv_obj_set_size(g_page_ctrl, 800, 480); lv_obj_set_pos(g_page_ctrl, 0, 0);

    g_page_wifi = bare_cont(lv_scr_act());
    lv_obj_set_size(g_page_wifi, 800, 480); lv_obj_set_pos(g_page_wifi, 0, 0);

    build_home_page();
    build_cam_page();
    build_ctrl_page();
    build_wifi_page();

    /* Start on HOME */
    lv_obj_add_flag(g_page_cam,  LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_page_ctrl, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_page_wifi, LV_OBJ_FLAG_HIDDEN);
}

/* ================================================================== */
/*  Signal / cleanup                                                   */
/* ================================================================== */
static void app_signal_handler(int s) { (void)s; g_app_running = 0; }

static void app_install_signal_handlers(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = app_signal_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

static void app_cleanup(void)
{
    cam_stop_all();
    if (g_cam_timer) lv_timer_pause(g_cam_timer);
    if (g_wifi_poll) lv_timer_del(g_wifi_poll);
    stream_thread_stop();
    cam_thread_deinit();
    uart_cleanup();
    gpio_cleanup();
}

/* ================================================================== */
/*  Display / input init                                               */
/* ================================================================== */
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
    const char *dev = getenv_default("LV_LINUX_EVDEV_POINTER_DEVICE", "/dev/input/event0");
    lv_indev_t *touch = lv_evdev_create(LV_INDEV_TYPE_POINTER, dev);
    lv_indev_set_display(touch, disp);
    LV_IMAGE_DECLARE(mouse_cursor_icon);
    lv_obj_t *cur = lv_image_create(lv_screen_active());
    lv_image_set_src(cur, &mouse_cursor_icon);
    lv_indev_set_cursor(touch, cur);
}
#endif

#if LV_USE_LINUX_FBDEV
static void lv_linux_disp_init(void)
{
    const char *dev = getenv_default("LV_LINUX_FBDEV_DEVICE", "/dev/fb0");
    lv_display_t *disp = lv_linux_fbdev_create();
    lv_display_set_rotation(NULL, LV_DISPLAY_ROTATION_90);
#if LV_USE_EVDEV
    lv_linux_init_input_pointer(disp);
#endif
    lv_linux_fbdev_set_file(disp, dev);
}
#elif LV_USE_LINUX_DRM
static void lv_linux_disp_init(void)
{
    const char *dev = getenv_default("LV_LINUX_DRM_CARD", "/dev/dri/card0");
    lv_display_t *disp = lv_linux_drm_create();
#if LV_USE_EVDEV
    lv_linux_init_input_pointer(disp);
#endif
    lv_linux_drm_set_file(disp, dev, -1);
}
#elif LV_USE_SDL
static void lv_linux_disp_init(void)
{
    lv_sdl_window_create(window_width, window_height);
}
#elif LV_USE_WAYLAND
#else
#error Unsupported configuration
#endif

#if LV_USE_WAYLAND == 0
void lv_linux_run_loop(void)
{
    uint32_t idle_time;
    while (g_app_running) {
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
        case 'f': fullscreen = true;
            if (!LV_USE_WAYLAND) { fprintf(stderr, "SDL no fullscreen\n"); exit(1); } break;
        case 'm': maximize = true;
            if (!LV_USE_WAYLAND) { fprintf(stderr, "SDL no maximize\n"); exit(1); } break;
        case 'w': window_width  = atoi(optarg); break;
        case 'h': window_height = atoi(optarg); break;
        case ':': fprintf(stderr, "Option -%c requires argument.\n", optopt); exit(1);
        case '?': fprintf(stderr, "Unknown option -%c.\n", optopt); exit(1);
        }
    }
}

/* ================================================================== */
/*  main                                                               */
/* ================================================================== */
int main(int argc, char **argv)
{
    configure_simulator(argc, argv);
    app_install_signal_handlers();

    lv_init();
    gpio_init();
    lv_linux_disp_init();

    cam_thread_init();
    stream_thread_start();

    build_ui();
    uart_init();

    lv_linux_run_loop();
    app_cleanup();
    return 0;
}
