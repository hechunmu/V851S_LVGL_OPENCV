#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Logical canvas size (LVGL rotation=90, physical fb 480x800 → logical 800x480) */
#define CAM_W 800   /* canvas width  (logical x) — full screen */
#define CAM_H 480   /* canvas height (logical y) */

void cam_thread_init(void);
void cam_thread_deinit(void);
void cam_capture_start(void);
void cam_capture_stop(void);

int cam_copy_frame(uint32_t *dst, int *last_seq);
int cam_is_open(void);
int cam_is_capturing(void);

int  cam_save_photo(const char *path);
int  cam_record_start(const char *path);
void cam_record_stop(void);
int  cam_is_recording(void);

#ifdef __cplusplus
}
#endif
