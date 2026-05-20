/**
 ******************************************************************************
 * @file    lesson_4_3.c
 * @author  百问科技
 * @version V1.0
 * @date    2024-9-23
 * @brief	Lesson 4_3 demo
 ******************************************************************************
 * Change Logs:
 * Date           Author          Notes
 * 2024-9-23     zhouyuebiao     First version
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

#if LV_USE_LESSON_DEMO_4_3

#include "lesson_4_3.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void flex_event_handler(lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lesson_4_3(void)
{
#if 1 // 4_3

#if 1   // Flex

    lv_obj_t * btn;
    lv_obj_t * label;
    uint32_t i;

    /////////////////////////////////////
    lv_obj_t * cont1 = lv_obj_create(lv_screen_active());
    lv_obj_set_size(cont1, lv_pct(100), lv_pct(20));
    lv_obj_align(cont1, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_flex_flow(cont1, LV_FLEX_FLOW_ROW);
    lv_obj_clear_flag(cont1, LV_OBJ_FLAG_SCROLLABLE);

    btn = lv_button_create(cont1);
    lv_obj_set_size(btn, LV_PCT(10), LV_PCT(100));

    label = lv_label_create(btn);
    lv_label_set_text(label, LV_SYMBOL_PLUS);
    lv_obj_center(label);

    lv_obj_t * cont_row = lv_obj_create(cont1);
    lv_obj_set_size(cont_row, lv_pct(90), lv_pct(100));
    lv_obj_align(cont_row, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_flex_flow(cont_row, LV_FLEX_FLOW_ROW);

    lv_obj_add_event_cb(btn, flex_event_handler, LV_EVENT_CLICKED, cont_row);

    for(i = 0; i < 5; i++) {
        /*Add items to the row*/
        btn = lv_button_create(cont_row);
        lv_obj_set_size(btn, 100, lv_pct(100));

        label = lv_label_create(btn);
        lv_label_set_text_fmt(label, "Item: %"LV_PRIu32"", i);
        lv_obj_center(label);
    }

    /////////////////////////////////////
    lv_obj_t * cont2 = lv_obj_create(lv_screen_active());
    lv_obj_set_size(cont2, lv_pct(100), lv_pct(70));
    lv_obj_align_to(cont2, cont1, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(cont2, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(cont2, LV_OBJ_FLAG_SCROLLABLE);

    btn = lv_button_create(cont2);
    lv_obj_set_size(btn, LV_PCT(10), 40);

    label = lv_label_create(btn);
    lv_label_set_text(label, LV_SYMBOL_PLUS);
    lv_obj_center(label);

    lv_obj_t * cont_row_wrap = lv_obj_create(cont2);
    lv_obj_set_size(cont_row_wrap, lv_pct(100), lv_pct(90));
    lv_obj_align(cont_row_wrap, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_flex_flow(cont_row_wrap, LV_FLEX_FLOW_ROW_WRAP);

    lv_obj_add_event_cb(btn, flex_event_handler, LV_EVENT_CLICKED, cont_row_wrap);

    for(i = 0; i < 9; i++) {
        /*Add items to the row*/
        btn = lv_button_create(cont_row_wrap);
        lv_obj_set_size(btn, lv_pct(30), lv_pct(10));

        label = lv_label_create(btn);
        lv_label_set_text_fmt(label, "Item: %"LV_PRIu32"", i);
        lv_obj_center(label);
    }
#else   // Grid
    static int32_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static int32_t row_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};

    lv_obj_t * cont = lv_obj_create(lv_screen_active());
    lv_obj_set_size(cont, lv_pct(100), lv_pct(100));
    lv_obj_center(cont);
    lv_obj_set_grid_dsc_array(cont, col_dsc, row_dsc);

    lv_obj_t * obj;

    obj = lv_obj_create(cont);
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 0, 4,
                             LV_GRID_ALIGN_STRETCH, 0, 1);

    obj = lv_obj_create(cont);
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 0, 1,
                             LV_GRID_ALIGN_STRETCH, 1, 3);
    
    obj = lv_obj_create(cont);
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 1, 3,
                             LV_GRID_ALIGN_STRETCH, 1, 2);    

    obj = lv_obj_create(cont);
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 1, 3,
                             LV_GRID_ALIGN_STRETCH, 3, 1);   
#endif

#endif

}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void flex_event_handler(lv_event_t * e)
{
    lv_obj_t * cont_row = lv_event_get_user_data(e);

    lv_obj_t * btn = lv_button_create(cont_row);
    lv_obj_set_size(btn, 100, LV_PCT(100));

    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text_fmt(label, "Item: %"LV_PRIu32"", lv_obj_get_child_count(cont_row)-1);
    lv_obj_center(label);

    lv_obj_scroll_to_view(btn, LV_ANIM_ON);
}



#endif /* LV_USE_LESSON_DEMO_4_3 */
