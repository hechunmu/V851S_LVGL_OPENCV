/**
 ******************************************************************************
 * @file    lesson_8_1.c
 * @author  百问科技
 * @version V1.0
 * @date    2024-10-09
 * @brief	Lesson 8_1 demo
 ******************************************************************************
 * Change Logs:
 * Date           Author          Notes
 * 2024-10-09     zhouyuebiao     First version
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

#if LV_USE_LESSON_DEMO_8_1

#include "lesson_8_1.h"

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
void lesson_8_1(void)
{
#if 1 // 8_1

#if 1 // 8-1-1
    LV_FONT_DECLARE(lv_my_fa_30);

    lv_obj_t * label1 = lv_label_create(lv_screen_active());
    lv_label_set_text(label1, "\xEF\x8B\xA7" "\xEF\x9A\xA7");
    lv_obj_set_style_text_font(label1, &lv_my_fa_30, 0);
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);
#elif 0 // 8-1-2

#if LV_USE_FREETYPE
    lv_font_t * font = lv_freetype_font_create("D:/100ask/FontAwesome/FontAwesome5-Solid+Brands+Regular.woff",
                                               LV_FREETYPE_FONT_RENDER_MODE_BITMAP,
                                               200,
                                               LV_FREETYPE_FONT_STYLE_NORMAL);

    if(!font) {
        LV_LOG_ERROR("freetype font create failed.");
        return;
    }

    /*Create style with the new font*/
    static lv_style_t style;
    lv_style_init(&style);
    //lv_style_set_text_font(&style, &lv_font_montserrat_14);
    lv_style_set_text_font(&style, font);
    lv_style_set_text_align(&style, LV_TEXT_ALIGN_CENTER);

    /*Create a label with the new style*/
    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_obj_add_style(label, &style, 0);
    lv_label_set_text(label, "\xEF\x8B\xA7" "\xEF\x9A\xA7" "\xEF\x97\x84");
    lv_obj_center(label);

    //lv_freetype_font_delete(font);
#endif

#endif

#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /* LV_USE_LESSON_DEMO_8_1 */
