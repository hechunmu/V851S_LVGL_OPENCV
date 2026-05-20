#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include "lib/include/awnn_lib.h"

using namespace cv;

#define FB_DEVICE "/dev/fb0"
#define MBV2_SSD 1

static const char *MODEL_PATH_TMP = "/tmp/mobilenet_v2_ssd.nb";

static const int TARGET_CLASS_IDS[] = {6, 7, 8, 12};

const char *class_name_from_index(int cls_idx)
{
    switch (cls_idx)
    {
    case 6:
        return "bus";
    case 7:
        return "car";
    case 8:
        return "cat";
    case 12:
        return "dog";
    default:
        return "unknown";
    }
}

struct Bbox_t
{
    int xmin, ymin, xmax, ymax;
    float score;
    int cls_idx;
};

bool comp(const Bbox_t &a, const Bbox_t &b)
{
    return a.score > b.score;
}

float intersection_area(const Bbox_t &a, const Bbox_t &b)
{
    Rect_<float> ra(a.xmin, a.ymin, a.xmax - a.xmin, a.ymax - a.ymin);
    Rect_<float> rb(b.xmin, b.ymin, b.xmax - b.xmin, b.ymax - b.ymin);
    return (ra & rb).area();
}

void nms_sorted_bboxes(const std::vector<Bbox_t> &boxes, std::vector<int> &picked, float threshold = 0.45f)
{
    picked.clear();
    std::vector<float> areas(boxes.size());
    for (size_t i = 0; i < boxes.size(); i++)
    {
        areas[i] = (boxes[i].xmax - boxes[i].xmin) * (boxes[i].ymax - boxes[i].ymin);
    }
    for (size_t i = 0; i < boxes.size(); i++)
    {
        const Bbox_t &a = boxes[i];
        bool keep = true;
        for (int j : picked)
        {
            const Bbox_t &b = boxes[j];
            float inter = intersection_area(a, b);
            float union_area = areas[i] + areas[j] - inter;
            if (inter / union_area > threshold)
            {
                keep = false;
                break;
            }
        }
        if (keep)
            picked.push_back(i);
    }
}

void get_input_data(const Mat &img, uint8_t *out, int h, int w)
{
    Mat rgb;
    cvtColor(img, rgb, COLOR_BGR2RGB);
    resize(rgb, rgb, Size(w, h));
    uint8_t *data = rgb.data;
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            for (int c = 0; c < 3; ++c)
                out[c * h * w + y * w + x] = data[y * w * 3 + x * 3 + c];
}

uint8_t *mbv2_ssd_preprocess(const Mat &img, int h, int w, int ch)
{
    uint8_t *buf = (uint8_t *)malloc(h * w * ch);
    get_input_data(img, buf, h, w);
    return buf;
}

Mat detect_ssd(Mat &bgr, float **output)
{
    float iou_threshold = 0.45;
    float conf_threshold = 0.5;
    const int inputH = 50, inputW = 50, outputClsSize = 21;
#if MBV2_SSD
    int output_dim_1 = 3000;
#else
    int output_dim_1 = 8732;
#endif
    int size0 = output_dim_1 * outputClsSize;
    int size1 = output_dim_1 * 4;

    std::vector<float> scores_data(output[0], output[0] + size0);
    std::vector<float> boxes_data(output[1], output[1] + size1);
    const float *scores = scores_data.data();
    const float *bboxes = boxes_data.data();

    float scale_w = bgr.cols / 50.0f;
    float scale_h = bgr.rows / 50.0f;

    std::vector<Bbox_t> BBox;
    for (int i = 0; i < output_dim_1; i++)
    {
        int max_idx = -1;
        float max_score = 0;
        for (int target_cls_idx : TARGET_CLASS_IDS)
        {
            float score = scores[i * outputClsSize + target_cls_idx];
            if (score > max_score)
            {
                max_score = score;
                max_idx = target_cls_idx;
            }
        }
        if (max_idx < 0 || max_score < conf_threshold)
            continue;

        int left = std::max(0, int(bboxes[i * 4] * scale_w * 6));
        int top = std::max(0, int(bboxes[i * 4 + 1] * scale_h * 6));
        int right = std::min(bgr.cols - 1, int(bboxes[i * 4 + 2] * scale_w * 6));
        int bottom = std::min(bgr.rows - 1, int(bboxes[i * 4 + 3] * scale_h * 6));

        if (right <= left || bottom <= top)
            continue;

        Bbox_t b{left, top, right, bottom, max_score, max_idx};
        BBox.emplace_back(b);
    }

    std::sort(BBox.begin(), BBox.end(), comp);
    std::vector<int> keep_index;
    nms_sorted_bboxes(BBox, keep_index, iou_threshold);

    float font_scale = std::max(0.5f, bgr.rows / 800.0f);
    int thickness = std::max(1, int(font_scale * 2.0f));

    for (int idx : keep_index)
    {
        const auto &box = BBox[idx];
        rectangle(bgr, Point(box.xmin, box.ymin), Point(box.xmax, box.ymax), Scalar(0, 0, 255), thickness);
        char text[128];
        snprintf(text, sizeof(text), "%s %.1f%%", class_name_from_index(box.cls_idx), box.score * 100);

        int baseline = 0;
        Size text_size = getTextSize(text, FONT_HERSHEY_SIMPLEX, font_scale, thickness, &baseline);
        int text_y = (box.ymin - 5 < text_size.height) ? box.ymin + text_size.height + 5 : box.ymin - 5;

        rectangle(bgr, Point(box.xmin, text_y - text_size.height),
                  Point(box.xmin + text_size.width, text_y + baseline), Scalar(0, 0, 255), FILLED);

        putText(bgr, text, Point(box.xmin, text_y), FONT_HERSHEY_SIMPLEX, font_scale, Scalar(255, 255, 255), thickness);
    }

    return bgr;
}

void convert_BGR_to_ARGB8888(const Mat &src, uint32_t *dst)
{
    for (int y = 0; y < src.rows; ++y)
    {
        for (int x = 0; x < src.cols; ++x)
        {
            Vec3b pixel = src.at<Vec3b>(y, x);
            dst[y * src.cols + x] = (0xFF << 24) | (pixel[2] << 16) | (pixel[1] << 8) | pixel[0];
        }
    }
}

const char *resolve_model_path()
{
    if (access(MODEL_PATH_TMP, R_OK) == 0)
        return MODEL_PATH_TMP;

    return nullptr;
}

bool open_camera_stream(VideoCapture &cap, Mat &probe_frame)
{
    static const Size candidate_sizes[] = {
        Size(640, 480),
        Size(1280, 720),
        Size(480, 800),
    };

    if (!cap.open(0))
        return false;

    for (const auto &candidate : candidate_sizes)
    {
        cap.set(CAP_PROP_FRAME_WIDTH, candidate.width);
        cap.set(CAP_PROP_FRAME_HEIGHT, candidate.height);
        usleep(200000);

        for (int attempt = 0; attempt < 5; ++attempt)
        {
            cap >> probe_frame;
            if (!probe_frame.empty())
            {
                std::cout << "摄像头采集分辨率: "
                          << probe_frame.cols << "x" << probe_frame.rows << std::endl;
                return true;
            }
            usleep(50000);
        }
    }

    return false;
}

int main()
{
    int fb_fd = open(FB_DEVICE, O_RDWR);
    fb_var_screeninfo vinfo;
    fb_fix_screeninfo finfo;
    ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);
    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
    if (vinfo.bits_per_pixel != 32)
    {
        std::cerr << "仅支持 ARGB8888 格式" << std::endl;
        return -1;
    }
    int screen_w = vinfo.xres;
    int screen_h = vinfo.yres;
    size_t screensize = screen_w * screen_h * 4;
    uint8_t *fbp = (uint8_t *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);

    VideoCapture cap;
    Mat frame, resized;
    if (!open_camera_stream(cap, frame))
    {
        std::cerr << "摄像头打开失败或 ISP 无法稳定出帧" << std::endl;
        return -1;
    }

    awnn_init(7 * 1024 * 1024);
    const char *model_path = resolve_model_path();
    if (model_path == nullptr)
    {
        std::cerr << "模型文件不存在，请先上传到 " << MODEL_PATH_TMP << std::endl;
        return -1;
    }

    std::cout << "使用模型: " << model_path << std::endl;
    Awnn_Context_t *ctx = awnn_create((char *)model_path);
    if (ctx == nullptr)
    {
        std::cerr << "模型加载失败: " << model_path << std::endl;
        return -1;
    }
    const int net_w = 300, net_h = 300, net_c = 3;

    std::vector<uint32_t> argb_buf(screen_w * screen_h);
    int empty_frame_count = 0;

    while (true)
    {
        cap >> frame;
        if (frame.empty())
        {
            ++empty_frame_count;
            if (empty_frame_count % 30 == 1)
                std::cerr << "摄像头取帧失败，检查 V4L2/ISP 配置" << std::endl;
            continue;
        }

        empty_frame_count = 0;

        Mat infer_input;
        resize(frame, infer_input, Size(net_w, net_h));
        uint8_t *input_data = mbv2_ssd_preprocess(infer_input, net_h, net_w, net_c);
        uint8_t *input_bufs[1] = {input_data};
        awnn_set_input_buffers(ctx, input_bufs);
        awnn_run(ctx);
        float **outputs = awnn_get_output_buffers(ctx);
        free(input_data);

        Mat detected_frame = detect_ssd(frame, outputs);
        resize(detected_frame, resized, Size(screen_w, screen_h));
        convert_BGR_to_ARGB8888(resized, argb_buf.data());

        for (int y = 300; y < screen_h && y < 800; ++y)
        {
            memcpy(fbp + y * finfo.line_length,
                   (uint8_t *)argb_buf.data() + y * screen_w * 4,
                   480 * 4);
        }

   

        usleep(10000);
    }

    awnn_destroy(ctx);
    cap.release();
    munmap(fbp, screensize);
    close(fb_fd);
    return 0;
}
