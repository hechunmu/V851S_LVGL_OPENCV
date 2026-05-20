/**
 ******************************************************************************
 * @file    lesson_4_2.c
 * @author  百问科技
 * @version V1.0
 * @date    2024-9-23
 * @brief	Lesson 4_2 demo
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

#if LV_USE_LESSON_DEMO_4_2

#include "lesson_4_2.h"

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
void lesson_4_2(void)
{
#if 1 // 4_2
    static int32_t col_dsc[] = {70, 70, 70, LV_GRID_TEMPLATE_LAST};
    static int32_t row_dsc[] = {50, 50, 50, LV_GRID_TEMPLATE_LAST};

    /*Create a container with grid*/
    lv_obj_t * cont = lv_obj_create(lv_screen_active());
    lv_obj_set_style_grid_column_dsc_array(cont, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, 0);
    lv_obj_set_size(cont, 300, 220);
    lv_obj_center(cont);
    lv_obj_set_layout(cont, LV_LAYOUT_GRID);

    lv_obj_t * label;
    lv_obj_t * obj;
    
#if 1
    obj = lv_button_create(cont);
    /*Stretch the cell horizontally and vertically too
        *Set span to 1 to make the cell 1 column/row sized*/
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_END, 0, 1,
                            LV_GRID_ALIGN_CENTER, 0, 3);
#else
    uint32_t i;
    for(i = 0; i < 9; i++) {
        uint8_t col = i % 3;
        uint8_t row = i / 3;

        obj = lv_button_create(cont);
        /*Stretch the cell horizontally and vertically too
            *Set span to 1 to make the cell 1 column/row sized*/
        //if(i == 1)
        //lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_CENTER, col, 1,
        //                        LV_GRID_ALIGN_STRETCH, row, 1);
        //else
        //lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, col, 1,
        //                        LV_GRID_ALIGN_STRETCH, row, 1);
        
        lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_START, col, 1,
                                LV_GRID_ALIGN_START, row, 1);

        //lv_obj_set_size(obj, 70, 50); // 验证填充修改

        label = lv_label_create(obj);
        lv_label_set_text_fmt(label, "c%d, r%d", col, row);
        lv_obj_center(label);
    }
    //                                   列                   行
    //lv_obj_set_grid_align(cont, LV_GRID_ALIGN_START, LV_GRID_ALIGN_CENTER);

    // 填充
    lv_obj_set_style_pad_row(cont, 0, 0);
    lv_obj_set_style_pad_column(cont, 0, 0);

#endif
#endif

}

/**********************
 *   STATIC FUNCTIONS
 **********************/


#endif /* LV_USE_LESSON_DEMO_4_2 */
