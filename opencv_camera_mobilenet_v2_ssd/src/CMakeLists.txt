cmake_minimum_required(VERSION 3.10)
project(opencv_camera_mobilenet_v2_ssd C CXX)

# 交叉编译工具链
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_C_COMPILER arm-openwrt-linux-gcc)
set(CMAKE_CXX_COMPILER arm-openwrt-linux-g++)

# 本地 OpenCV + SDK 路径
set(OPENCV_DIR /home/ubuntu/opencv-mobile-4.11.0-yuzuki-lizard)
set(STAGING_DIR /home/ubuntu/v851s-tina/out/v851s/lizard/openwrt/staging_dir/target)
set(AWNN_LOCAL_DIR ${CMAKE_SOURCE_DIR}/lib)

# 头文件路径
include_directories(
    ${STAGING_DIR}/usr/include
    ${STAGING_DIR}/usr/include/viplite-driver
    ${OPENCV_DIR}/include
    ${OPENCV_DIR}/include/opencv4
    ${AWNN_LOCAL_DIR}/include
)

# 库文件路径
link_directories(
    ${STAGING_DIR}/usr/lib
    ${OPENCV_DIR}/lib
    ${AWNN_LOCAL_DIR}
)

# 源文件
file(GLOB SRC_FILES *.cpp *.c)

# 可执行文件
add_executable(${PROJECT_NAME} ${SRC_FILES})

# 链接库
target_link_libraries(${PROJECT_NAME}
    ${AWNN_LOCAL_DIR}/libawnn.a
    -lVIPlite
    -lVIPuser
    -ldl
    -lpthread
    -lm
    -lstdc++

    -Wl,--start-group
    ${OPENCV_DIR}/lib/libopencv_core.a
    ${OPENCV_DIR}/lib/libopencv_features2d.a
    ${OPENCV_DIR}/lib/libopencv_highgui.a
    ${OPENCV_DIR}/lib/libopencv_imgproc.a
    ${OPENCV_DIR}/lib/libopencv_photo.a
    ${OPENCV_DIR}/lib/libopencv_video.a
    -Wl,--end-group
)
