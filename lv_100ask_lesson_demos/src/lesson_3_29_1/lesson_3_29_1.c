/**
 ******************************************************************************
 * @file    lesson_3_29_1.c
 * @author  百问科技
 * @version V1.0
 * @date    2024-9-10
 * @brief	Lesson 3_29_1 demo
 ******************************************************************************
 * Change Logs:
 * Date           Author          Notes
 * 2024-9-10     zhouyuebiao     First version
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

#if LV_USE_LESSON_DEMO_3_29_1

#include "lesson_3_29_1.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    lv_obj_t * page;
    lv_dir_t dir;
} lv_100ask_page_can_switch_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void tileview_event_handler_3_29_1(lv_event_t * e);
static void scroll_switch_page_event_3_29_2(lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lesson_3_29_1(void)
{
#if 1 // 3_29_1
    lv_obj_t * tv = lv_tileview_create(lv_screen_active());

    /*Tile1*/
    lv_obj_t * tile1 = lv_tileview_add_tile(tv, 0, 0, LV_DIR_BOTTOM);
    lv_obj_t * label = lv_label_create(tile1);
    lv_label_set_text(label, "Scroll down");
    lv_obj_center(label);

    /*Tile2*/
    lv_obj_t * tile2 = lv_tileview_add_tile(tv, 0, 1, LV_DIR_TOP | LV_DIR_RIGHT);

    lv_obj_t * btn = lv_button_create(tile2);

    label = lv_label_create(btn);
    lv_label_set_text(label, "Scroll up or right");

    lv_obj_set_size(btn, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_center(btn);

     /*Tile3*/
    lv_obj_t * tile3 = lv_tileview_add_tile(tv, 1, 1, LV_DIR_LEFT);
    lv_obj_t * list = lv_list_create(tile3);
    lv_obj_set_size(list, LV_PCT(100), LV_PCT(100));

    lv_list_add_button(list, NULL, "One");
    lv_list_add_button(list, NULL, "Two");
    lv_list_add_button(list, NULL, "Three");
    lv_list_add_button(list, NULL, "Four");
    lv_list_add_button(list, NULL, "Five");
    lv_list_add_button(list, NULL, "Six");
    lv_list_add_button(list, NULL, "Seven");
    lv_list_add_button(list, NULL, "Eight");
    lv_list_add_button(list, NULL, "Nine");
    lv_list_add_button(list, NULL, "Ten");

    //
    //lv_tileview_set_tile(tv, tile2, LV_ANIM_OFF);
    //lv_tileview_set_tile_by_index(tv, 1, 1, LV_ANIM_ON);

    lv_obj_add_event_cb(tv, tileview_event_handler_3_29_1, LV_EVENT_VALUE_CHANGED, NULL);

#elif 0 // 3_29_2

    lv_obj_t * label;

    // page1
    lv_obj_t * page1 = lv_obj_create(lv_screen_active());
    lv_obj_set_size(page1, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(page1, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_center(page1);

    label = lv_label_create(page1);
    lv_label_set_text(label, "PAGE1");
    lv_obj_center(label);

    // page2
    lv_obj_t * page2 = lv_obj_create(lv_screen_active());
    lv_obj_set_size(page2, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(page2, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_obj_center(page2);

    label = lv_label_create(page2);
    lv_label_set_text(label, "PAGE2");
    lv_obj_center(label);

    ///
    static lv_100ask_page_can_switch_t page1_can_switch;
    page1_can_switch.dir = LV_DIR_LEFT;
    page1_can_switch.page = page2;

    static lv_100ask_page_can_switch_t page2_can_switch;
    page2_can_switch.dir = LV_DIR_RIGHT;
    page2_can_switch.page = page1;

    ///
    //lv_obj_add_flag(page1, LV_OBJ_FLAG_HIDDEN);
    //lv_obj_add_flag(page2, LV_OBJ_FLAG_HIDDEN);

    //lv_obj_remove_flag(page1, LV_OBJ_FLAG_GESTURE_BUBBLE);
    //lv_obj_remove_flag(page2, LV_OBJ_FLAG_GESTURE_BUBBLE);
    //lv_obj_add_event_cb(page1, scroll_switch_page_event_3_29_2, LV_EVENT_GESTURE, &page1_can_switch);
    //lv_obj_add_event_cb(page2, scroll_switch_page_event_3_29_2, LV_EVENT_GESTURE, &page2_can_switch);
    
#endif

}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void tileview_event_handler_3_29_1(lv_event_t * e)
{
    lv_obj_t * tv = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t * tv_index = lv_tileview_get_tile_active(tv);
        LV_LOG_USER("%d", lv_obj_get_index(tv_index));
    }
}


static void scroll_switch_page_event_3_29_2(lv_event_t * e)
{
  //lv_obj_t * obj = lv_event_get_current_target(e);
  lv_obj_t * cur_page = lv_event_get_target(e);
  lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
  lv_100ask_page_can_switch_t * page_can_switch = lv_event_get_user_data(e);

  if(dir == page_can_switch->dir)
  {
    lv_obj_add_flag(cur_page, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(page_can_switch->page, LV_OBJ_FLAG_HIDDEN);
  }
}

#endif /* LV_USE_LESSON_DEMO_3_29_1 */
