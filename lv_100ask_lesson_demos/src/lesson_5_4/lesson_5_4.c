/**
 ******************************************************************************
 * @file    lesson_5_4.c
 * @author  百问科技
 * @version V1.0
 * @date    2024-9-26
 * @brief	Lesson 5_4 demo
 ******************************************************************************
 * Change Logs:
 * Date           Author          Notes
 * 2024-9-26     zhouyuebiao     First version
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

#if LV_USE_LESSON_DEMO_5_4

#include "lesson_5_4.h"


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
void lesson_5_4(void)
{
#if 0 // 5_4
    lv_color_t bg_color = lv_palette_lighten(LV_PALETTE_RED, 5);
    lv_color_t fg_color = lv_palette_darken(LV_PALETTE_RED, 4);

    lv_obj_t * qr = lv_qrcode_create(lv_screen_active());
    
    /*Set data*/
    const char * data = "https://www.100ask.netfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfsefhrgdthjgfgasdrfgdfbgdfsh";
    lv_qrcode_update(qr, data, strlen(data));
    lv_obj_center(qr);

    //lv_obj_set_size(qr, 200, 200);
    lv_qrcode_set_size(qr, 200);
    lv_qrcode_update(qr, data, strlen(data));

    lv_qrcode_set_dark_color(qr, fg_color);
    lv_qrcode_set_light_color(qr, bg_color);
    lv_qrcode_update(qr, data, strlen(data));

    /*Add a border with bg_color*/
    lv_obj_set_style_border_color(qr, bg_color, 0);
    lv_obj_set_style_border_width(qr, 5, 0);

#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/


#endif /* LV_USE_LESSON_DEMO_5_4 */
