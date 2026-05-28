#include "cam_thread.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <vector>

using namespace cv;

#define CAM_PHYS_ROWS 500
#define CAM_PHYS_COLS 480

/* ------------------------------------------------------------------ */
/*  Shared state between camera thread and LVGL main thread            */
/* ------------------------------------------------------------------ */
static uint32_t        s_buf[CAM_H * CAM_W];
static int             s_seq       = 0;
static pthread_mutex_t s_mutex     = PTHREAD_MUTEX_INITIALIZER;

/* Thread lifecycle */
static pthread_t       s_tid       = 0;
static volatile int    s_exit      = 0;   /* set to 1 to kill thread   */
static volatile int    s_capture   = 0;   /* set to 1 to deliver frames */
static volatile int    s_cam_open  = 0;   /* 1 after camera opened OK  */

/* Recording state */
static FILE           *s_rec_file     = NULL;
static volatile int    s_recording    = 0;
static pthread_mutex_t s_rec_mutex    = PTHREAD_MUTEX_INITIALIZER;

/* ------------------------------------------------------------------ */
/*  Frame transform: resize + 90° rotation into canvas layout          */
/* ------------------------------------------------------------------ */
static void write_frame(const Mat &frame)
{
    static uint32_t tmp[CAM_H * CAM_W];
    Mat scaled;
    resize(frame, scaled, Size(CAM_PHYS_COLS, CAM_PHYS_ROWS));

    for (int fr = 0; fr < CAM_PHYS_ROWS; fr++) {
        int cx = (CAM_W - 1) - fr;
        const Vec3b *row = scaled.ptr<Vec3b>(fr);
        for (int fc = 0; fc < CAM_PHYS_COLS; fc++) {
            const Vec3b &p = row[fc];
            tmp[fc * CAM_W + cx] =
                (0xFFu << 24) |
                ((uint32_t)p[2] << 16) |
                ((uint32_t)p[1] <<  8) |
                 (uint32_t)p[0];
        }
    }

    pthread_mutex_lock(&s_mutex);
    memcpy(s_buf, tmp, sizeof(s_buf));
    s_seq++;
    pthread_mutex_unlock(&s_mutex);

    if (s_recording) {
        std::vector<uchar> jpeg_buf;
        cv::imencode(".jpg", scaled, jpeg_buf);
        pthread_mutex_lock(&s_rec_mutex);
        if (s_rec_file)
            fwrite(jpeg_buf.data(), 1, jpeg_buf.size(), s_rec_file);
        pthread_mutex_unlock(&s_rec_mutex);
    }
}

/* ------------------------------------------------------------------ */
/*  Camera thread: opens camera ONCE, never closes until s_exit        */
/* ------------------------------------------------------------------ */
static void *cam_thread_func(void *)
{
    VideoCapture cap;
    bool opened = false;
    for (int idx = 0; idx < 3 && !opened; idx++) {
        fprintf(stderr, "[cam] trying /dev/video%d\n", idx);
        if (cap.open(idx)) opened = true;
        else cap.release();
    }
    if (!opened) {
        fprintf(stderr, "[cam] 摄像头打开失败\n");
        return NULL;
    }

    cap.set(CAP_PROP_FRAME_WIDTH,  640);
    cap.set(CAP_PROP_FRAME_HEIGHT, 480);
    usleep(200000);

    /* probe first frame */
    Mat frame;
    for (int i = 0; i < 10 && frame.empty(); i++) {
        cap >> frame;
        if (frame.empty()) usleep(50000);
    }
    if (frame.empty()) {
        fprintf(stderr, "[cam] 无法获取首帧\n");
        cap.release();
        return NULL;
    }

    s_cam_open = 1;
    fprintf(stderr, "[cam] 摄像头已就绪 %dx%d, 进入待机\n", frame.cols, frame.rows);

    int empty_cnt = 0;
    while (!s_exit) {
        cap >> frame;
        if (frame.empty()) {
            if (++empty_cnt % 30 == 1)
                fprintf(stderr, "[cam] 取帧失败\n");
            usleep(10000);
            continue;
        }
        empty_cnt = 0;

        /* Only process frame if capture mode is active */
        if (s_capture)
            write_frame(frame);
        /* else: frame is read and discarded to keep ISP buffer queue drained */
    }

    cap.release();
    fprintf(stderr, "[cam] 线程退出\n");
    return NULL;
}

/* ------------------------------------------------------------------ */
/*  Public C API                                                        */
/* ------------------------------------------------------------------ */
extern "C" void cam_thread_init(void)
{
    s_exit    = 0;
    s_capture = 0;
    s_cam_open = 0;
    pthread_create(&s_tid, NULL, cam_thread_func, NULL);
    fprintf(stderr, "[cam] 线程已创建\n");
}

extern "C" void cam_thread_deinit(void)
{
    s_exit = 1;
    if (s_tid) {
        pthread_join(s_tid, NULL);
        s_tid = 0;
    }
}

extern "C" void cam_capture_start(void)
{
    s_seq     = 0;   /* reset so consumer sees new frames immediately */
    s_capture = 1;
    fprintf(stderr, "[cam] 开始显示\n");
}

extern "C" void cam_capture_stop(void)
{
    s_capture = 0;
    fprintf(stderr, "[cam] 停止显示\n");
}

extern "C" int cam_copy_frame(uint32_t *dst, int *last_seq)
{
    pthread_mutex_lock(&s_mutex);
    int updated = (s_seq != *last_seq);
    if (updated) {
        memcpy(dst, s_buf, CAM_W * CAM_H * sizeof(uint32_t));
        *last_seq = s_seq;
    }
    pthread_mutex_unlock(&s_mutex);
    return updated;
}

extern "C" int cam_is_open(void)
{
    return s_cam_open;
}

<<<<<<< HEAD
extern "C" int cam_is_capturing(void)
{
    return s_capture;
=======
extern "C" int cam_save_photo(const char *path)
{
    static uint32_t photo_buf[CAM_H * CAM_W];
    pthread_mutex_lock(&s_mutex);
    memcpy(photo_buf, s_buf, sizeof(photo_buf));
    pthread_mutex_unlock(&s_mutex);

    /* Convert XRGB8888 canvas buffer to BGR Mat */
    Mat bgr(CAM_H, CAM_W, CV_8UC3);
    for (int y = 0; y < CAM_H; y++) {
        Vec3b *row = bgr.ptr<Vec3b>(y);
        const uint32_t *src = photo_buf + y * CAM_W;
        for (int x = 0; x < CAM_W; x++) {
            uint32_t px = src[x];
            row[x] = Vec3b(px & 0xFF, (px >> 8) & 0xFF, (px >> 16) & 0xFF);
        }
    }

    if (!cv::imwrite(path, bgr)) {
        fprintf(stderr, "[cam] 照片保存失败: %s\n", path);
        return -1;
    }
    fprintf(stderr, "[cam] 照片已保存: %s\n", path);
    return 0;
}

extern "C" int cam_record_start(const char *path)
{
    pthread_mutex_lock(&s_rec_mutex);
    if (s_rec_file) { fclose(s_rec_file); s_rec_file = NULL; }
    s_rec_file = fopen(path, "wb");
    if (!s_rec_file) {
        pthread_mutex_unlock(&s_rec_mutex);
        fprintf(stderr, "[cam] 录像文件打开失败: %s\n", path);
        return -1;
    }
    s_recording = 1;
    pthread_mutex_unlock(&s_rec_mutex);
    fprintf(stderr, "[cam] 开始录像: %s\n", path);
    return 0;
}

extern "C" void cam_record_stop(void)
{
    pthread_mutex_lock(&s_rec_mutex);
    s_recording = 0;
    if (s_rec_file) { fclose(s_rec_file); s_rec_file = NULL; }
    pthread_mutex_unlock(&s_rec_mutex);
    fprintf(stderr, "[cam] 录像已停止\n");
}

extern "C" int cam_is_recording(void)
{
    return s_recording;
>>>>>>> sync 2026-05-27_21-59-26
}
