/**
 ******************************************************************************
 * @file    lesson_3_28_1.c
 * @author  百问科技
 * @version V1.0
 * @date    2024-9-09
 * @brief	Lesson 3_28_1 demo
 ******************************************************************************
 * Change Logs:
 * Date           Author          Notes
 * 2024-9-09     zhouyuebiao     First version
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

#if LV_USE_LESSON_DEMO_3_28_1

#include "lesson_3_28_1.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void tabview_event_handler_3_28_1(lv_event_t * e);
static void scroll_begin_event_3_28_5(lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lesson_3_28_1(void)
{
#if 1 // 3_28_1
    lv_obj_t * tabview;

    tabview = lv_tabview_create(lv_screen_active());

    //lv_tabview_set_tab_bar_position(tabview, LV_DIR_LEFT);
    //lv_tabview_set_tab_bar_size(tabview, 200);

    /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
    lv_obj_t * tab1 = lv_tabview_add_tab(tabview, "Tab 1");
    lv_obj_t * tab2 = lv_tabview_add_tab(tabview, "Tab 2");
    lv_obj_t * tab3 = lv_tabview_add_tab(tabview, "Tab 3");

    lv_tabview_rename_tab(tabview, 0, "Tab 11");
    lv_tabview_rename_tab(tabview, 1, "Tab 22");
    lv_tabview_rename_tab(tabview, 2, "Tab 33");

    //lv_tabview_set_active(tabview, 2, LV_ANIM_ON);

    lv_obj_add_event_cb(tabview, tabview_event_handler_3_28_1, LV_EVENT_VALUE_CHANGED, NULL);

#elif 0 // 3_28_2
    lv_obj_t * tabview;

    tabview = lv_tabview_create(lv_screen_active());

    /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
    lv_obj_t * tab1 = lv_tabview_add_tab(tabview, "Tab 1");
    lv_obj_t * tab2 = lv_tabview_add_tab(tabview, "Tab 2");
    lv_obj_t * tab3 = lv_tabview_add_tab(tabview, "Tab 3");

    // tab1
    lv_obj_t * label;

    lv_obj_t * btn1 = lv_button_create(tab1);
    lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -40);
    lv_obj_remove_flag(btn1, LV_OBJ_FLAG_PRESS_LOCK);

    label = lv_label_create(btn1);
    lv_label_set_text(label, "Button");
    lv_obj_center(label);

    // tab2
    lv_obj_t * btn2 = lv_button_create(tab2);
    lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 40);
    lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_height(btn2, LV_SIZE_CONTENT);

    label = lv_label_create(btn2);
    lv_label_set_text(label, "Toggle");
    lv_obj_center(label);

    // tab3
    LV_IMAGE_DECLARE(animimg001);
    LV_IMAGE_DECLARE(animimg002);
    LV_IMAGE_DECLARE(animimg003);

    static const lv_image_dsc_t * anim_imgs[3] = {
        &animimg001,
        & animimg002,
        & animimg003,
    };

    lv_obj_t * animimg0 = lv_animimg_create(tab3);
    lv_obj_center(animimg0);
    lv_animimg_set_src(animimg0, (const void **) anim_imgs, 3);
    lv_animimg_set_duration(animimg0, 1000);
    lv_animimg_set_repeat_count(animimg0, LV_ANIM_REPEAT_INFINITE);
    lv_animimg_start(animimg0);
#elif 0 // 3_28_3
    lv_obj_t * tabview;

    tabview = lv_tabview_create(lv_screen_active());

    /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
    lv_obj_t * tab1 = lv_tabview_add_tab(tabview, "Tab 1");
    lv_obj_t * tab2 = lv_tabview_add_tab(tabview, "Tab 2");
    lv_obj_t * tab3 = lv_tabview_add_tab(tabview, "Tab 3");

    lv_obj_t * tab_bar = lv_tabview_get_tab_bar(tabview);
    lv_obj_add_flag(tab_bar, LV_OBJ_FLAG_HIDDEN);

#elif 0 // 3_28_4
    lv_obj_t * tabview;

    LV_IMAGE_DECLARE(img_cogwheel_argb);
    lv_obj_t * img1 = lv_image_create(lv_screen_active());
    lv_obj_set_size(img1, lv_pct(100), lv_pct(100));
    lv_image_set_src(img1, &img_cogwheel_argb);
    lv_obj_align(img1, LV_ALIGN_CENTER, 0, 0);
    lv_image_set_inner_align(img1, LV_IMAGE_ALIGN_TILE);

    tabview = lv_tabview_create(lv_screen_active());

    //lv_obj_set_style_bg_color(tabview, lv_palette_main(LV_PALETTE_GREEN), 0);
    //lv_obj_set_style_bg_color(tabview, lv_palette_main(LV_PALETTE_BLUE_GREY), 0);
    //lv_obj_set_style_bg_opa(tabview, LV_OPA_TRANSP, 0);

    /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
    lv_obj_t * tab1 = lv_tabview_add_tab(tabview, "Tab 1");
    lv_obj_t * tab2 = lv_tabview_add_tab(tabview, "Tab 2");
    lv_obj_t * tab3 = lv_tabview_add_tab(tabview, "Tab 3");

    //lv_obj_t * tab_bar = lv_tabview_get_tab_bar(tabview);
    //lv_obj_add_flag(tab_bar, LV_OBJ_FLAG_HIDDEN);

    // tab1
    lv_obj_t * label;

    lv_obj_t * btn1 = lv_button_create(tab1);
    lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -40);
    lv_obj_remove_flag(btn1, LV_OBJ_FLAG_PRESS_LOCK);

    label = lv_label_create(btn1);
    lv_label_set_text(label, "Button");
    lv_obj_center(label);

    // tab2
    lv_obj_t * btn2 = lv_button_create(tab2);
    lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 40);
    lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_height(btn2, LV_SIZE_CONTENT);

    label = lv_label_create(btn2);
    lv_label_set_text(label, "Toggle");
    lv_obj_center(label);

    // tab3
    LV_IMAGE_DECLARE(animimg001);
    LV_IMAGE_DECLARE(animimg002);
    LV_IMAGE_DECLARE(animimg003);

    static const lv_image_dsc_t * anim_imgs[3] = {
        &animimg001,
        & animimg002,
        & animimg003,
    };

    lv_obj_t * animimg0 = lv_animimg_create(tab3);
    lv_obj_center(animimg0);
    lv_animimg_set_src(animimg0, (const void **) anim_imgs, 3);
    lv_animimg_set_duration(animimg0, 1000);
    lv_animimg_set_repeat_count(animimg0, LV_ANIM_REPEAT_INFINITE);
    lv_animimg_start(animimg0);

#elif 0 // 3_28_5
    lv_obj_t * tabview;

    tabview = lv_tabview_create(lv_screen_active());

    //lv_tabview_set_tab_bar_position(tabview, LV_DIR_LEFT);

    /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
    lv_obj_t * tab1 = lv_tabview_add_tab(tabview, "Tab 1");
    lv_obj_t * tab2 = lv_tabview_add_tab(tabview, "Tab 2");
    lv_obj_t * tab3 = lv_tabview_add_tab(tabview, "Tab 3");

    //lv_obj_add_event_cb(lv_tabview_get_content(tabview), scroll_begin_event_3_28_5, LV_EVENT_SCROLL_END, NULL);
#endif

}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void tabview_event_handler_3_28_1(lv_event_t * e)
{
    lv_obj_t * tb = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        uint32_t tab_index = lv_tabview_get_tab_active(tb);
        LV_LOG_USER("%d", tab_index);
    }
}

static void scroll_begin_event_3_28_5(lv_event_t * e)
{
    lv_obj_t * cont = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);

    lv_obj_t * tv = lv_obj_get_parent(cont);

    if(lv_event_get_code(e) == LV_EVENT_SCROLL_END) {
        lv_indev_t * indev = lv_indev_active();
        if(indev && lv_indev_get_state(indev) == LV_INDEV_STATE_PRESSED) {
            return;
        }

        int32_t tb_index;
        lv_coord_t s;

        lv_point_t p;
        lv_obj_get_scroll_end(cont, &p);

        // 注意这里的写法，与视频中编写的的不一样！
        if((lv_obj_get_style_flex_flow(lv_tabview_get_tab_bar(tv), 0) == LV_FLEX_FLOW_COLUMN) ||\
           (lv_obj_get_style_flex_flow(lv_tabview_get_tab_bar(tv), 0) == LV_FLEX_FLOW_COLUMN_REVERSE)) {
            
            s = lv_obj_get_scroll_y(cont);
            //printf("s: %d\n", s);
            if(s < 0) tb_index = lv_tabview_get_tab_count(tv) - 1;
            if((tb_index == (lv_tabview_get_tab_count(tv) - 1)) && (s > p.y)) tb_index = 0;
        }
        else {

            s = lv_obj_get_scroll_x(cont);
            //printf("s: %d\n", s);
            if(s < 0) tb_index = lv_tabview_get_tab_count(tv) - 1;
            if((tb_index == (lv_tabview_get_tab_count(tv) - 1)) && (s > p.x)) tb_index = 0;
            
        }

        bool new_tab = false;
        if(tb_index != (int32_t)lv_tabview_get_tab_active(tv)) new_tab = true;

        /*If not scrolled by an indev set the tab immediately*/
        if(lv_indev_active()) {
            lv_tabview_set_active(tv, tb_index, LV_ANIM_ON);
        }
        else {
            lv_tabview_set_active(tv, tb_index, LV_ANIM_OFF);
        }

        if(new_tab) lv_obj_send_event(tv, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

#endif /* LV_USE_LESSON_DEMO_3_28_1 */
