#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Logical canvas size (LVGL rotation=90, physical fb 480x800 → logical 800x480) */
#define CAM_W 500   /* canvas width  (logical x) */
#define CAM_H 480   /* canvas height (logical y) */

/*
 * Lifecycle:
 *   cam_thread_init()    - call once at startup; opens camera, enters drain mode
 *   cam_capture_start()  - begin delivering frames to the canvas buffer
 *   cam_capture_stop()   - pause delivery (camera stays open, ISP keeps running)
 *   cam_thread_deinit()  - call on program exit
 *
 * The camera is NEVER closed between start/stop to avoid ISP driver crashes on
 * V851S/V853 hardware that does not support reopen after release.
 */
void cam_thread_init(void);
void cam_thread_deinit(void);
void cam_capture_start(void);
void cam_capture_stop(void);

/*
 * Copy the latest frame into dst (CAM_W*CAM_H XRGB8888 pixels).
 * last_seq: caller tracks last seen sequence number (init to 0).
 * Returns 1 if a new frame was copied, 0 if no change.
 */
int cam_copy_frame(uint32_t *dst, int *last_seq);

/* Returns 1 if camera was successfully opened */
int cam_is_open(void);

<<<<<<< HEAD
/* Returns 1 if capture is active (camON pressed) */
int cam_is_capturing(void);
=======
/*
 * Photo: encode current frame as JPEG and write to path.
 * Returns 0 on success, -1 on failure.
 */
int cam_save_photo(const char *path);

/*
 * Video recording (concatenated MJPEG stream, readable by ffmpeg/VLC):
 *   cam_record_start(path) - open file and start appending JPEG frames
 *   cam_record_stop()      - flush and close
 *   cam_is_recording()     - 1 while recording
 */
int  cam_record_start(const char *path);
void cam_record_stop(void);
int  cam_is_recording(void);
>>>>>>> sync 2026-05-27_21-59-26

#ifdef __cplusplus
}
#endif
