# Usage:
# cmake -DCMAKE_TOOLCHAIN_FILE=./user_cross_compile_setup.cmake -B build -S .
# make  -C build -j

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(tools /home/ubuntu/100ask/T113s4_Lvgl9-Class_TinaSDK-V1/prebuilt/rootfsbuilt/arm/toolchain-sunxi-glibc-gcc-830/toolchain)
set(CMAKE_C_COMPILER ${tools}/bin/arm-openwrt-linux-gnueabi-gcc)
set(CMAKE_CXX_COMPILER ${tools}/bin/arm-openwrt-linux-gnueabi-g++)

# If necessary, set STAGING_DIR
# if not work, please try(in shell command): export STAGING_DIR=/home/ubuntu/100ask/T113s4_Lvgl9-Class_TinaSDK-V1/out/t113_s4/100ask_lvgl9/openwrt/staging_dir/target
set(ENV{STAGING_DIR} "/home/ubuntu/100ask/T113s4_Lvgl9-Class_TinaSDK-V1/out/t113_s4/100ask_lvgl9/openwrt/staging_dir/target")

