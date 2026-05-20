#
CROSS_COMPILE_PATH ?= /home/ubuntu/tina-v853-open/prebuilt/rootfsbuilt/arm/toolchain-sunxi-musl-gcc-830/toolchain/bin
export PATH := $(CROSS_COMPILE_PATH):$(PATH)
# 交叉编译工具链
CC  := arm-openwrt-linux-gcc
CXX := arm-openwrt-linux-g++

# 本地 OpenCV 路径（非 STAGING）
OPENCV_DIR := /home/ubuntu/opencv-mobile-4.11.0-yuzuki-lizard
STAGING_DIR := /home/ubuntu/v851s-tina/out/v851s/lizard/openwrt/staging_dir/target
# Include 头文件
INCLUDES += -I$(STAGING_DIR)/usr/include
INCLUDES += -I$(STAGING_DIR)/usr/include/viplite-driver
INCLUDES += -I$(OPENCV_DIR)/include
INCLUDES += -I$(OPENCV_DIR)/include/opencv4

# 库文件路径
LIBS += -L$(STAGING_DIR)/usr/lib          # awnn / VIP / 用户库
LIBS += -L$(OPENCV_DIR)/lib               # 本地 opencv
LIBS += -Llib                             # 自带 libawnn.so/.a

# 最终目标文件
BIN := opencv_camera_mobilenet_v2_ssd

# 源码与对象文件
SRCS := ${wildcard *.c} ${wildcard *.cpp}
OBJS := $(addsuffix .o, $(basename $(SRCS)))

# 编译选项
CFLAGS += -O2
LFLAGS += 

# 编译规则
all: $(BIN)


%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f *.o $(BIN) *~
