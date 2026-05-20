/**
 ******************************************************************************
 * @file    lesson_3_32_1.c
 * @author  百问科技
 * @version V1.0
 * @date    2024-9-19
 * @brief	Lesson 3_32_1 demo
 ******************************************************************************
 * Change Logs:
 * Date           Author          Notes
 * 2024-9-19     zhouyuebiao     First version
 ******************************************************************************
 * @attention
 *
 * Copyright (C) 2008-2024 深圳百问网科技有限公司<https://www.100ask.net/>
 * All rights reserved
 *
 * 代码配套的视频教程：
 *      B站：   https://www.bilibili.com/video/BV1WE421K75k
 *      百问网：https://fnwcn.xetslk.com/s/39njGj
 *      淘宝：  https://detail.tmall.com/item.htm?id=779667445604
 *
 * 本程序遵循MIT协议, 请遵循协议！
 * 免责声明: 百问网编写的文档, 仅供学员学习使用, 可以转发或引用(请保留作者信息),禁止用于商业用途！
 * 免责声明: 百问网编写的程序, 仅供学习参考，假如被用于商业用途, 但百问网不承担任何后果！
 *
 * 百问网学习平台   : https://www.100ask.net
 * 百问网交流社区   : https://forums.100ask.net
 * 百问网LVGL文档   : https://lvgl.100ask.net
 * 百问网官方B站    : https://space.bilibili.com/275908810
 * 百问网官方淘宝   : https://100ask.taobao.com
 * 百问网微信公众号 ：百问科技 或 baiwenkeji
 * 联系我们(E-mail):  support@100ask.net 或 fae_100ask@163.com
 *
 *                             版权所有，盗版必究。
 ******************************************************************************
 */

/*********************
 *      INCLUDES
 *********************/
#include "../../lv_100ask_lesson_demos.h"

#if LV_USE_LESSON_DEMO_3_32_1

#include "lesson_3_32_1.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lesson_3_32_1(void)
{
#if 0 // 3_32_1
    extern const uint8_t lv_example_lottie_approve[];
    extern const size_t lv_example_lottie_approve_size;

    lv_obj_t * lottie = lv_lottie_create(lv_screen_active());

    /*For GPUs and special alignment/strid setting use a draw_buf instead*/
    LV_DRAW_BUF_DEFINE(draw_buf, 64, 64, LV_COLOR_FORMAT_ARGB8888);
    lv_lottie_set_draw_buf(lottie, &draw_buf);

    //lv_lottie_set_src_data(lottie, lv_example_lottie_approve, lv_example_lottie_approve_size);
    lv_lottie_set_src_file(lottie, "lvgl/examples/widgets/lottie/lv_example_lottie_approve.json");
    //lv_lottie_set_src_file(lottie, "lv_example_lottie_approve.json");

    lv_obj_center(lottie);

#elif 0 // 3_32_2
    extern const uint8_t my_example_lottie_approve[];
    extern const size_t my_example_lottie_approve_size;

    lv_obj_t * lottie = lv_lottie_create(lv_screen_active());

    /*For GPUs and special alignment/strid setting use a draw_buf instead*/
    LV_DRAW_BUF_DEFINE(draw_buf, 64, 64, LV_COLOR_FORMAT_ARGB8888);
    lv_lottie_set_draw_buf(lottie, &draw_buf);

    lv_lottie_set_src_data(lottie, my_example_lottie_approve, my_example_lottie_approve_size);

    lv_obj_center(lottie);
#endif

}

/**********************
 *   STATIC FUNCTIONS
 **********************/


#endif /* LV_USE_LESSON_DEMO_3_32_1 */
