#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Start MJPEG HTTP server on port 8090.
 * Clients can connect and pull MJPEG stream.
 * Frames come from cam_thread's shared buffer.
 * After WiFi is up, also notifies the relay server.
 */
void stream_thread_start(void);
void stream_thread_stop(void);

#ifdef __cplusplus
}
#endif
