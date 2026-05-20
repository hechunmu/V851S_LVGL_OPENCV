/**
 ******************************************************************************
 * @file    lesson_3_25_1.c
 * @author  百问科技
 * @version V1.0
 * @date    2024-9-06
 * @brief	Lesson 3_25_1 demo
 ******************************************************************************
 * Change Logs:
 * Date           Author          Notes
 * 2024-9-06     zhouyuebiao     First version
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

#if LV_USE_LESSON_DEMO_3_25_1

#include "lesson_3_25_1.h"

/*********************
 *      DEFINES
 *********************/


/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void event_handler(lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 **********************/


/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lesson_3_25_1(void)
{
#if 1 // 3_25_1
    LV_IMAGE_DECLARE(imagebutton_left);
    LV_IMAGE_DECLARE(imagebutton_right);
    LV_IMAGE_DECLARE(imagebutton_mid);

    static lv_style_t style_pr;
    lv_style_init(&style_pr);
    lv_style_set_image_recolor_opa(&style_pr, LV_OPA_30);
    lv_style_set_image_recolor(&style_pr, lv_color_black());
    lv_style_set_transform_width(&style_pr, 20);

    lv_obj_t * imagebutton = lv_imagebutton_create(lv_screen_active());
    lv_imagebutton_set_src(imagebutton, LV_IMAGEBUTTON_STATE_RELEASED, &imagebutton_left, &imagebutton_mid,
                           &imagebutton_right);

    lv_obj_set_size(imagebutton, 100, 50);
    lv_obj_align(imagebutton, LV_ALIGN_CENTER, 0, 0);    

    lv_obj_add_style(imagebutton, &style_pr, LV_STATE_PRESSED); 
    //lv_obj_add_style(imagebutton, &style_pr, LV_STATE_CHECKED);    

    //lv_imagebutton_set_state(imagebutton, LV_IMAGEBUTTON_STATE_DISABLED);
    //lv_imagebutton_set_state(imagebutton, LV_IMAGEBUTTON_STATE_PRESSED);                  

    lv_obj_add_flag(imagebutton, LV_OBJ_FLAG_CHECKABLE);

    lv_obj_add_event_cb(imagebutton, event_handler, LV_EVENT_ALL, NULL);
    
#elif 0 // 3_25_3
    LV_IMAGE_DECLARE(imagebutton_left);
    LV_IMAGE_DECLARE(imagebutton_right);
    LV_IMAGE_DECLARE(imagebutton_mid);

    static lv_style_t style_pr;
    lv_style_init(&style_pr);
    lv_style_set_image_recolor_opa(&style_pr, LV_OPA_30);
    lv_style_set_image_recolor(&style_pr, lv_color_black());
    lv_style_set_transform_width(&style_pr, 20);

    lv_obj_t * imagebutton = lv_imagebutton_create(lv_screen_active());
    lv_imagebutton_set_src(imagebutton, LV_IMAGEBUTTON_STATE_RELEASED, &imagebutton_left, &imagebutton_mid,
                           &imagebutton_right);
    //lv_imagebutton_set_src(imagebutton, LV_IMAGEBUTTON_STATE_PRESSED, &imagebutton_right, &imagebutton_mid, &imagebutton_left);
    //lv_imagebutton_set_src(imagebutton, LV_IMAGEBUTTON_STATE_PRESSED, &imagebutton_mid, &imagebutton_left, &imagebutton_left);

    lv_obj_set_size(imagebutton, 100, 50);
    lv_obj_align(imagebutton, LV_ALIGN_CENTER, 0, 0);    

    //lv_obj_add_style(imagebutton, &style_pr, LV_STATE_PRESSED);     
#endif

}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
    }
    else if(code == LV_EVENT_VALUE_CHANGED) {
        LV_LOG_USER("Toggled");
    }
}

#endif /* LV_USE_LESSON_DEMO_3_25_1 */
