#
# Makefile
#
CROSS_COMPILE_PATH ?= /home/ubuntu/tina-v853-open/prebuilt/rootfsbuilt/arm/toolchain-sunxi-musl-gcc-830/toolchain/bin
export PATH := $(CROSS_COMPILE_PATH):$(PATH)
CC     = arm-openwrt-linux-gcc
CXX    = arm-openwrt-linux-g++
STRIP  = arm-openwrt-linux-strip

LVGL_DIR_NAME ?= lvgl
LVGL_DIR      ?= .

OPENCV_DIR    = /home/ubuntu/opencv-mobile-4.11.0-yuzuki-lizard
STAGING_DIR   = /home/ubuntu/tina-v853-open/out/v851s/lizard/openwrt/staging_dir/target

WARNINGS := -Wall -Wshadow -Wundef -Wmissing-prototypes -Wno-discarded-qualifiers \
            -Wextra -Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith \
            -fno-strict-aliasing -Wno-error=cpp -Wuninitialized -Wmaybe-uninitialized \
            -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits \
            -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wno-cast-qual \
            -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar \
            -Wformat-security -Wno-ignored-qualifiers -Wno-error=pedantic \
            -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion \
            -Wclobbered -Wdeprecated -Wempty-body -Wshift-negative-value \
            -Wstack-usage=2048 -Wno-unused-value -std=gnu99

CFLAGS ?= -Os -g0 -I$(LVGL_DIR)/ $(WARNINGS) -ffunction-sections -fdata-sections
CFLAGS += -I$(OPENCV_DIR)/include -I$(OPENCV_DIR)/include/opencv4
CFLAGS += -I$(STAGING_DIR)/usr/include

# C++ flags: same base but c++17 instead of gnu99, strip C-only warning flags
CXXFLAGS = -Os -g0 -I$(LVGL_DIR)/ \
           -I$(OPENCV_DIR)/include -I$(OPENCV_DIR)/include/opencv4 \
           -I$(STAGING_DIR)/usr/include \
           -Wall -Wno-unused-parameter -Wno-shadow \
           -fno-strict-aliasing -ffunction-sections -fdata-sections -std=c++17

LDFLAGS = -lm -lpthread -lrt -ldl -lstdc++ \
          -Wl,--gc-sections \
          -L$(STAGING_DIR)/usr/lib \
          -Wl,--start-group \
          $(OPENCV_DIR)/lib/libopencv_core.a \
          $(OPENCV_DIR)/lib/libopencv_highgui.a \
          $(OPENCV_DIR)/lib/libopencv_imgproc.a \
          -ljpeg \
          -Wl,--end-group

BIN         = main
BUILD_DIR     = ./build
BUILD_OBJ_DIR = $(BUILD_DIR)/obj
BUILD_BIN_DIR = $(BUILD_DIR)/bin

prefix  ?= /usr
bindir  ?= $(prefix)/bin

# Main C source
MAINSRC = ./main.c

include $(LVGL_DIR)/lvgl/lvgl.mk
include $(LVGL_DIR)/lv_100ask_lesson_demos/lv_100ask_lesson_demos.mk

CSRCS   += $(LVGL_DIR)/mouse_cursor_icon.c
CSRCS   += ./app_gpio.c
CSRCS   += ./app_uart.c

# Camera thread + MJPEG streaming thread (C++)
CXXSRCS += ./cam_thread.cpp
CXXSRCS += ./stream_thread.cpp

OBJEXT ?= .o

AOBJS   = $(ASRCS:.S=$(OBJEXT))
COBJS   = $(CSRCS:.c=$(OBJEXT))
CXXOBJS = $(CXXSRCS:.cpp=$(OBJEXT))
MAINOBJ = $(MAINSRC:.c=$(OBJEXT))

SRCS   = $(ASRCS) $(CSRCS) $(CXXSRCS) $(MAINSRC)
OBJS   = $(AOBJS) $(COBJS) $(CXXOBJS) $(MAINOBJ)
TARGET = $(addprefix $(BUILD_OBJ_DIR)/, $(patsubst ./%, %, $(OBJS)))

all: default

$(BUILD_OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "CXX $<"

$(BUILD_OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "CC $<"

$(BUILD_OBJ_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "CC $<"

default: $(TARGET)
	@mkdir -p $(dir $(BUILD_BIN_DIR)/)
	$(CXX) -o $(BUILD_BIN_DIR)/$(BIN) $(TARGET) $(LDFLAGS)
	$(STRIP) --strip-all $(BUILD_BIN_DIR)/$(BIN)
	@echo "Binary size: $$(du -sh $(BUILD_BIN_DIR)/$(BIN) | cut -f1)"

clean:
	rm -rf $(BUILD_DIR)

install:
	install -d $(DESTDIR)$(bindir)
	install $(BUILD_BIN_DIR)/$(BIN) $(DESTDIR)$(bindir)

uninstall:
	$(RM) -r $(addprefix $(DESTDIR)$(bindir)/,$(BIN))
