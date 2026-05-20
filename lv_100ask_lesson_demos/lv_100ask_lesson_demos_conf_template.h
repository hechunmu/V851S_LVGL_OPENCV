/**
 * @file lv_100ask_lesson_demos_conf.h
 * Configuration file for v9.1.0
 *
 */
/*
 * COPY THIS FILE AS lv_100ask_lesson_demos_conf.h
 */

/* clang-format off */
#if 0 /*Set it to "1" to enable the content*/

#ifndef LV_100ASK_LESSON_DEMOS_CONF_H
#define LV_100ASK_LESSON_DEMOS_CONF_H


/*******************
 * GENERAL SETTING
 *******************/

/*********************
 * DEMO USAGE
 *********************/

/*Show some widget*/

/* Section 1 */
#define LV_USE_LESSON_DEMO_2_3_1      1     // 基础对象(lv_obj)，"Hello, LVGL!"
#define LV_USE_LESSON_DEMO_2_4_1      1     // 基础对象的大小(size)
#define LV_USE_LESSON_DEMO_2_5_1      1     // 基础对象的位置(position)
#define LV_USE_LESSON_DEMO_2_6_1      1     // 基础对象的盒子模型(border-box)
#define LV_USE_LESSON_DEMO_2_7_1      1     // 基础对象的样式(styles)
#define LV_USE_LESSON_DEMO_2_8_1      1     // 基础对象的事件(events)、事件冒泡
#define LV_USE_LESSON_DEMO_2_9_1      1     // LVGL的定时器(lv_timer)-入门
#define LV_USE_LESSON_DEMO_3_1_1      1     // 标签控件(lv_label)的用法
#define LV_USE_LESSON_DEMO_3_2_1      1     // 按钮控件(lv_button)的用法
#define LV_USE_LESSON_DEMO_3_3_1      1     // 进度条控件(lv_bar)的用法
#define LV_USE_LESSON_DEMO_3_4_1      1     // 滑动条控件(lv_slider)的用法
#define LV_USE_LESSON_DEMO_3_5_1      1     // 圆弧控件(lv_arc)的用法
#define LV_USE_LESSON_DEMO_3_6_1      1     // 环形加载器控件(lv_spinner)的用法
#define LV_USE_LESSON_DEMO_3_7_1      1     // 矩阵按钮控件(lv_btnmatrix)的用法
#define LV_USE_LESSON_DEMO_3_8_1      1     // 列表控件(lv_list)的用法
#define LV_USE_LESSON_DEMO_3_9_1      1     // 开关控件(lv_switch)的用法
#define LV_USE_LESSON_DEMO_3_10_1     1     // 复选框控件(lv_checkbox)的用法
#define LV_USE_LESSON_DEMO_3_11_1     1     // 线条控件(lv_line)的用法
#define LV_USE_LESSON_DEMO_3_12_1     1     // LED控件(lv_led)的用法
#define LV_USE_LESSON_DEMO_3_13_1     1     // 下拉列表控件(lv_dropdown)的用法
#define LV_USE_LESSON_DEMO_3_14_1     1     // 滚轮控件(lv_roller)的用法
#define LV_USE_LESSON_DEMO_3_15_1     1     // 消息框控件lv_msgbox的用法
#define LV_USE_LESSON_DEMO_3_16_1     1     // 富文本控件(lv_span)的用法
#define LV_USE_LESSON_DEMO_3_17_1     1     // 文本框控件(lv_textarea)的用法
#define LV_USE_LESSON_DEMO_3_18_1     1     // 键盘控件(lv_keyboard)的用法
#define LV_USE_LESSON_DEMO_3_19_1     1     // 微调框控件(lv_spinbox)的用法
#define LV_USE_LESSON_DEMO_3_20_1     1     // 表格控件(lv_table)的用法
#define LV_USE_LESSON_DEMO_3_21_1     1     // 标尺控件lv_scale的用法
#define LV_USE_LESSON_DEMO_3_22_1     1     // 图表控件(lv_chart)的用法
#define LV_USE_LESSON_DEMO_3_23_1     1     // 日历控件(lv_calendar)的用法
#define LV_USE_LESSON_DEMO_3_24_1     1     // 图像控件(lv_img)的用法
#define LV_USE_LESSON_DEMO_3_25_1     1     // 图像按钮控件(lv_imagebutton)的用法
#define LV_USE_LESSON_DEMO_3_26_1     1     // 动画图像控件(lv_animimg)用法
#define LV_USE_LESSON_DEMO_3_27_1     1     // 画布控件(lv_canvas)的用法
#define LV_USE_LESSON_DEMO_3_28_1     1     // 选项卡控件(lv_tabview)的用法
#define LV_USE_LESSON_DEMO_3_29_1     1     // 平铺视图控件(lv_tileview)的用法
#define LV_USE_LESSON_DEMO_3_30_1     1     // 菜单控件(lv_menu)的用法
#define LV_USE_LESSON_DEMO_3_31_1     1     // 窗口控件(lv_win)的用法
#define LV_USE_LESSON_DEMO_3_32_1     1     // Lottie控件(lv_lottie)的用法
#define LV_USE_LESSON_DEMO_4_1        1     // Flex（弹性）布局的用法
#define LV_USE_LESSON_DEMO_4_2        1     // Grid（弹性）布局的用法
#define LV_USE_LESSON_DEMO_4_3        1     // Flex（弹性）和Grid（网格）布局的区别
#define LV_USE_LESSON_DEMO_5_1        1     // LVGL文件系统模块的用法
#define LV_USE_LESSON_DEMO_5_2        1     // 通过LVGL文件系统模块的显示图片文件
#define LV_USE_LESSON_DEMO_5_3        1     // FreeType控件（lv_freetype）的用法
#define LV_USE_LESSON_DEMO_5_4        1     // 二维码控件(lv_qrcode)的用法
#define LV_USE_LESSON_DEMO_5_5        1     // 条形码控件(lv_barcode)的用法
#define LV_USE_LESSON_DEMO_6_1        1     // 文件资源管理器控件（lv_file_explorer）的用法
#define LV_USE_LESSON_DEMO_6_2        1     // 拼音输入法控件（lv_ime_pinyin）的用法
#define LV_USE_LESSON_DEMO_6_3        1     // 快照控件（lv_snapshot）的用法
#define LV_USE_LESSON_DEMO_6_4        1     // Monkey测试（压力测试）的用法
#define LV_USE_LESSON_DEMO_7_1        1     // 更进一步理解LVGL的定时器
#define LV_USE_LESSON_DEMO_7_2        1     // 动画的用法
#define LV_USE_LESSON_DEMO_7_3        1     // 动画时间线的用法
#define LV_USE_LESSON_DEMO_7_4        1     // 动画的本质是定时器处理
#define LV_USE_LESSON_DEMO_7_5        1     // 为什么创建出来的不同控件会不一样？
#define LV_USE_LESSON_DEMO_7_6        1     // 物理按键代替触摸（鼠标模仿编码器）
#define LV_USE_LESSON_DEMO_7_7        1     // Windows HOOK拦截键盘输入模拟按键输入
#define LV_USE_LESSON_DEMO_8_1        1     // 基于FontAwesome 添加新SYMBOLS
#define LV_USE_LESSON_DEMO_8_2        1     // 调试方法之LV_LOG
#define LV_USE_LESSON_DEMO_8_3        1     // 调试方法之LV_SYSMON(system monitor)
#define LV_USE_LESSON_DEMO_9_1        1     // 移植适配 lv_lib_100ask
#define LV_USE_LESSON_DEMO_9_2        1     // 移植适配 lv_100ask_ui
#define LV_USE_LESSON_DEMO_10_1       1     // LVGL的编码风格
#define LV_USE_LESSON_DEMO_10_2       1     // 如何阅读Changelog
#define LV_USE_LESSON_DEMO_10_3       1     // 如何阅读Roadmap
#define LV_USE_LESSON_DEMO_10_4       1     // 如何参与LVGL开发-为LVGL库做贡献



#endif /*LV_100ASK_LESSON_DEMOS_CONF_H*/

#endif /*End of "Content enable"*/

