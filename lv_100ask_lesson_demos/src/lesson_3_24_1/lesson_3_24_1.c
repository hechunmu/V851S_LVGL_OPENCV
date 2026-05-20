/**
 ******************************************************************************
 * @file    lesson_3_24_1.c
 * @author  百问科技
 * @version V1.0
 * @date    2024-8-30
 * @brief	Lesson 3_24_1 demo
 ******************************************************************************
 * Change Logs:
 * Date           Author          Notes
 * 2024-8-30     zhouyuebiao     First version
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

#if LV_USE_LESSON_DEMO_3_24_1

#include "lesson_3_24_1.h"

/*********************
 *      DEFINES
 *********************/


/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void ofs_y_anim(void * img, int32_t v);
static void set_angle(void * img, int32_t v);
static void set_scale(void * img, int32_t v);
static void img_clicked_event_handler(lv_event_t * e);
static void img_clicked_event_handler_3_24_5(lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 **********************/


/**********************
 *      MACROS
 **********************/
LV_IMAGE_DECLARE(img_cogwheel_argb);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lesson_3_24_1(void)
{
#if 1 // 3_24_1
    LV_IMAGE_DECLARE(img_cogwheel_argb);
    lv_obj_t * img = lv_image_create(lv_screen_active());
    lv_image_set_src(img, &img_cogwheel_argb);
    //lv_image_set_src(img, LV_SYMBOL_OK "Some text");
    lv_obj_center(img);

    //lv_obj_set_style_image_recolor_opa(img, LV_OPA_100, 0);
    //lv_obj_set_style_image_recolor(img, lv_color_hex(0xff0000), 0);

#if 0
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, img);
    lv_anim_set_exec_cb(&a, ofs_y_anim);
    lv_anim_set_values(&a, 0, 100);
    lv_anim_set_duration(&a, 3000);
    lv_anim_set_playback_duration(&a, 500);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);
#endif
    //lv_image_set_scale(img, 512);

#if 0
    lv_image_set_pivot(img, 0, 0);    /*Rotate around the top left corner*/
    
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, img);
    lv_anim_set_exec_cb(&a, set_angle);
    lv_anim_set_values(&a, 0, 3600);
    lv_anim_set_duration(&a, 5000);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);

    lv_anim_set_exec_cb(&a, set_scale);
    lv_anim_set_values(&a, 128, 256);
    lv_anim_set_playback_duration(&a, 3000);
    lv_anim_start(&a);
#endif

    lv_obj_set_style_bg_opa(img, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(img, lv_color_hex(0xff0000), 0);

    lv_obj_set_size(img, 250, 100);
    lv_image_set_inner_align(img, LV_IMAGE_ALIGN_TILE);

    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, img_clicked_event_handler, LV_EVENT_CLICKED, NULL);

#elif 0 // 3_24_3
    LV_IMAGE_DECLARE(img_app_icon_about_us);
    lv_obj_t * img = lv_image_create(lv_screen_active());
    lv_image_set_src(img, &img_app_icon_about_us);
    lv_obj_center(img);

#elif 0 // 3_24_4
    lv_obj_t * img = lv_image_create(lv_screen_active());
    lv_image_set_src(img, "D:/100ask/lvgl_100ask_course_v9/part1/02_codes/lv_sim_codeblocks_win/lv_100ask_lesson_demos/assets/img_app_icon_about_us.png");
    lv_obj_center(img);

#elif 0 // 3_24_5
    lv_obj_t * img = lv_image_create(lv_screen_active());
    lv_image_set_src(img, "D:/100ask/lvgl_100ask_course_v9/part1/02_codes/lv_sim_codeblocks_win/lv_100ask_lesson_demos/assets/img_app_icon_about_us.png");
    lv_obj_center(img);

    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, img_clicked_event_handler_3_24_5, LV_EVENT_CLICKED, NULL);
#endif

}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void img_clicked_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
    }
}

static void img_clicked_event_handler_3_24_5(lv_event_t * e)
{
    static uint8_t flag = 0;
    lv_obj_t * img = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
        if(flag == 1)
        {
            flag = 0;
            lv_image_set_src(img, "D:/100ask/lvgl_100ask_course_v9/part1/02_codes/lv_sim_codeblocks_win/lv_100ask_lesson_demos/assets/img_app_icon_about_us.png");
        }
        else if(flag == 0)
        {
            flag = 1;
            lv_image_set_src(img, &img_cogwheel_argb);
        }        
    }   
}

static void ofs_y_anim(void * img, int32_t v)
{
    lv_image_set_offset_x(img, v);
    lv_image_set_offset_y(img, v);
}

static void set_angle(void * img, int32_t v)
{
    lv_image_set_rotation(img, v);
}

static void set_scale(void * img, int32_t v)
{
    lv_image_set_scale(img, v);
}

#endif /* LV_USE_LESSON_DEMO_3_24_1 */
