/**
 ******************************************************************************
 * @file    lesson_3_30_1.c
 * @author  百问科技
 * @version V1.0
 * @date    2024-9-11
 * @brief	Lesson 3_30_1 demo
 ******************************************************************************
 * Change Logs:
 * Date           Author          Notes
 * 2024-9-11     zhouyuebiao     First version
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

#if LV_USE_LESSON_DEMO_3_30_1

#include "lesson_3_30_1.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void menu_main_back_event_handler(lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lesson_3_30_1(void)
{
#if 1 // 3_30_1
    lv_obj_t * menu = lv_menu_create(lv_screen_active());

    lv_obj_set_style_bg_color(menu, lv_color_darken(lv_obj_get_style_bg_color(menu, 0), 50), 0);

    lv_menu_set_mode_header(menu, LV_MENU_HEADER_BOTTOM_FIXED);
    lv_menu_set_mode_root_back_button(menu, LV_MENU_ROOT_BACK_BUTTON_ENABLED);
    lv_obj_add_event_cb(menu, menu_main_back_event_handler, LV_EVENT_CLICKED, menu);

    /*Create sub pages*/
    lv_obj_t * main_page;
    lv_obj_t * sub_page;
    lv_obj_t * btn1;
    lv_obj_t * btn2;
    lv_obj_t * label;
    
    // main page
    main_page = lv_menu_page_create(menu, "100ASK1");

    lv_obj_t *menu_cont = lv_menu_cont_create(main_page);
    btn1 = lv_button_create(menu_cont);

    lv_obj_t *menu_separator = lv_menu_separator_create(main_page);
    label = lv_label_create(menu_separator);
    lv_label_set_text(label, "----------------------");

    lv_obj_t *menu_section = lv_menu_section_create(main_page);
    btn1 = lv_button_create(menu_section);

    //lv_obj_t * cont = lv_menu_cont_create(main_page);
    label = lv_label_create(btn1);
    lv_label_set_text(label, "Item 3 (Click me!)");

    // sub page
    sub_page = lv_menu_page_create(menu, "100ASK2");
    btn2 = lv_button_create(sub_page);

    // 设置页面跳转，最终都能返回到主页面
    lv_menu_set_load_page_event(menu, btn1, sub_page);
    lv_menu_set_load_page_event(menu, btn2, main_page);

    // 设置主页面
    lv_menu_set_page(menu, main_page);
    //lv_menu_set_sidebar_page(menu, main_page);


#elif 1 // 3_30_3
    lv_obj_t * menu = lv_menu_create(lv_screen_active());

    lv_obj_set_style_bg_color(menu, lv_color_darken(lv_obj_get_style_bg_color(menu, 0), 50), 0);

    lv_obj_t *main_header = lv_menu_get_main_header(menu);
    //lv_obj_add_flag(main_header, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_height(main_header, 0);
    
#endif

}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void menu_main_back_event_handler(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    lv_obj_t * menu = lv_event_get_user_data(e);

    if(lv_menu_back_button_is_root(menu, obj)) {
        lv_obj_t * mbox1 = lv_msgbox_create(NULL);
        lv_msgbox_add_title(mbox1, "Hello");
        lv_msgbox_add_text(mbox1, "Root back btn click.");
        lv_msgbox_add_close_button(mbox1);
    }
}



#endif /* LV_USE_LESSON_DEMO_3_30_1 */
