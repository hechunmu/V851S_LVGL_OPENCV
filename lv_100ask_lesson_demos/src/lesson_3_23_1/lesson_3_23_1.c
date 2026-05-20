/**
 ******************************************************************************
 * @file    lesson_3_23_1.c
 * @author  百问科技
 * @version V1.0
 * @date    2024-8-29
 * @brief	Lesson 3_23_1 demo
 ******************************************************************************
 * Change Logs:
 * Date           Author          Notes
 * 2024-8-29     zhouyuebiao     First version
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

#if LV_USE_LESSON_DEMO_3_23_1

#include "lesson_3_23_1.h"

/*********************
 *      DEFINES
 *********************/


/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void event_handler_3_23_1(lv_event_t * e);
static void calendar_btnm_draw_task_added_event_cb_3_23_3(lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 **********************/


/**********************
 *      MACROS
 **********************/
#define LV_CALENDAR_CTRL_TODAY      LV_BUTTONMATRIX_CTRL_CUSTOM_1
#define LV_CALENDAR_CTRL_HIGHLIGHT  LV_BUTTONMATRIX_CTRL_CUSTOM_2

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lesson_3_23_1(void)
{
#if 1 // 3_23_1
    lv_obj_t  * calendar = lv_calendar_create(lv_screen_active());
    lv_obj_set_size(calendar, lv_pct(50), lv_pct(50));
    lv_calendar_set_today_date(calendar, 2024, 8, 30);
    lv_calendar_set_showed_date(calendar, 2024, 8);

    /*Highlight a few days*/
    static lv_calendar_date_t highlighted_days[3];       /*Only its pointer will be saved so should be static*/
    highlighted_days[0].year = 2024;
    highlighted_days[0].month = 8;
    highlighted_days[0].day = 6;

    highlighted_days[1].year = 2024;
    highlighted_days[1].month = 8;
    highlighted_days[1].day = 15;

    highlighted_days[2].year = 2024;
    highlighted_days[2].month = 8;
    highlighted_days[2].day = 22;

    lv_calendar_set_highlighted_dates(calendar, highlighted_days, 3);

    static const char * day_names[7] = {"111", "222", "333", "444", "555", "666", "777"};
    lv_calendar_set_day_names(calendar, day_names);

    static const char * years_list = "2024\n2023\n2022\n2021\n2020\n2019";
    lv_calendar_header_dropdown_set_year_list(calendar, years_list);

    lv_calendar_header_dropdown_create(calendar);
    //lv_calendar_header_arrow_create(calendar);


    //lv_obj_add_event_cb(calendar, event_handler_3_23_1, LV_EVENT_VALUE_CHANGED, NULL);
    //lv_calendar_set_chinese_mode(calendar, true);
    //lv_obj_set_style_text_font(calendar, &lv_font_simsun_14_cjk, LV_PART_MAIN);

#elif 1 // 3_23_3
    lv_obj_t  * calendar = lv_calendar_create(lv_screen_active());
    lv_obj_set_size(calendar, lv_pct(50), lv_pct(50));
    lv_calendar_set_today_date(calendar, 2024, 8, 30);
    lv_calendar_set_showed_date(calendar, 2024, 8);

    // 注意： 这里需放在离 lv_calendar_create 尽可能近的位置，这里如果你放在最后（154行之后）会有问题
    lv_obj_t * calendar_btnm = lv_obj_get_child(calendar, 0);
    lv_obj_add_event_cb(calendar_btnm, calendar_btnm_draw_task_added_event_cb_3_23_3, LV_EVENT_DRAW_TASK_ADDED, NULL);

    /*Highlight a few days*/
    static lv_calendar_date_t highlighted_days[3];       /*Only its pointer will be saved so should be static*/
    highlighted_days[0].year = 2024;
    highlighted_days[0].month = 8;
    highlighted_days[0].day = 6;

    highlighted_days[1].year = 2024;
    highlighted_days[1].month = 8;
    highlighted_days[1].day = 15;

    highlighted_days[2].year = 2024;
    highlighted_days[2].month = 8;
    highlighted_days[2].day = 22;

    lv_calendar_set_highlighted_dates(calendar, highlighted_days, 3);

    static const char * day_names[7] = {"111", "222", "333", "444", "555", "666", "777"};
    lv_calendar_set_day_names(calendar, day_names);

    static const char * years_list = "2024\n2023\n2022\n2021\n2020\n2019";
    lv_calendar_header_dropdown_set_year_list(calendar, years_list);

    lv_calendar_header_dropdown_create(calendar);
    //lv_calendar_header_arrow_create(calendar);

    lv_obj_add_event_cb(calendar, event_handler_3_23_1, LV_EVENT_VALUE_CHANGED, NULL);
    lv_calendar_set_chinese_mode(calendar, true);
    lv_obj_set_style_text_font(calendar, &lv_font_simsun_14_cjk, LV_PART_MAIN);

#endif

}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void event_handler_3_23_1(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_current_target(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        lv_calendar_date_t date;
        if(lv_calendar_get_pressed_date(obj, &date)) {
            //LV_LOG_USER("Clicked date: %02d.%02d.%d", date.day, date.month, date.year);
            LV_LOG_USER("Clicked date: %02d.%02d.%d", date.year, date.month, date.day);
        }
    }
}


static void calendar_btnm_draw_task_added_event_cb_3_23_3(lv_event_t * e)
{
    lv_obj_t * calendar = lv_event_get_target(e);
    lv_obj_t * calendar_btnm = lv_event_get_current_target(e);
    //lv_draw_task_t * draw_task = lv_event_get_param(e);
    lv_draw_task_t * draw_task = lv_event_get_draw_task(e);

    lv_draw_dsc_base_t * base_dsc = lv_draw_task_get_draw_dsc(draw_task);
    if(base_dsc->part == LV_PART_ITEMS) {
        lv_draw_fill_dsc_t * fill_draw_dsc = lv_draw_task_get_fill_dsc(draw_task);
        lv_draw_border_dsc_t * border_draw_dsc = lv_draw_task_get_border_dsc(draw_task);

        if(lv_buttonmatrix_has_button_ctrl(calendar_btnm, base_dsc->id1, LV_CALENDAR_CTRL_HIGHLIGHT))
        {
            if(border_draw_dsc) border_draw_dsc->color = lv_color_hex(0xff0000);
            //if(fill_draw_dsc) fill_draw_dsc->opa = LV_OPA_40;
            if(fill_draw_dsc) fill_draw_dsc->color = lv_color_hex(0xfff000);

        }

        if(lv_buttonmatrix_has_button_ctrl(calendar_btnm, base_dsc->id1, LV_CALENDAR_CTRL_TODAY))
        {
            if(border_draw_dsc) border_draw_dsc->opa = LV_OPA_COVER;
            if(border_draw_dsc) border_draw_dsc->color = lv_color_hex(0x00ff00);
            if(border_draw_dsc) border_draw_dsc->width += 1;

            if(lv_draw_task_get_type(draw_task) == LV_DRAW_TASK_TYPE_FILL) {
                LV_IMAGE_DECLARE(img_star);
                lv_image_header_t header;
                lv_result_t res = lv_image_decoder_get_info(&img_star, &header);
                if(res != LV_RESULT_OK) return;

                lv_area_t a;
                a.x1 = 0;
                a.x2 = header.w - 1;
                a.y1 = 0;
                a.y2 = header.h - 1;
                lv_area_t draw_task_area;
                lv_draw_task_get_area(draw_task, &draw_task_area);
                lv_area_align(&draw_task_area, &a, LV_ALIGN_CENTER, 0, 0);

                lv_draw_image_dsc_t img_draw_dsc;
                lv_draw_image_dsc_init(&img_draw_dsc);
                img_draw_dsc.src = &img_star;
                img_draw_dsc.recolor = lv_color_black();
                //if(pressed) img_draw_dsc.recolor_opa = LV_OPA_30;

                lv_draw_image(base_dsc->layer, &img_draw_dsc, &a);
            }
        }
    }
}

#endif /* LV_USE_LESSON_DEMO_3_23_1 */
