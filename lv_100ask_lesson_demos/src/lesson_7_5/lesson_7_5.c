/**
 ******************************************************************************
 * @file    lesson_7_5.c
 * @author  百问科技
 * @version V1.0
 * @date    2024-10-08
 * @brief	Lesson 7_5 demo
 ******************************************************************************
 * Change Logs:
 * Date           Author          Notes
 * 2024-10-08     zhouyuebiao     First version
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

#if LV_USE_LESSON_DEMO_7_5

#include "lesson_7_5.h"


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
void lesson_7_5(void)
{
#if 1 // 7_5
    // 无代码，分析课
    //lv_obj_t * btn1 = lv_button_create(lv_screen_active());

    /*
    怎么创建主题
    lv_display_create
    lv_theme_default_init
    	theme->base.apply_cb = theme_apply;
		style_init  // 在 lv_conf.h 中裁剪掉不需要的控件或者使用不同的主题可以节省资源，也会影响这里的初始化
    ///////////////////////////////////////
    怎么使用主题
    lv_xxx_create
        lv_obj_class_create_obj
        lv_obj_class_init_obj
                lv_theme_apply
                    apply_theme_recursion
                        apply_theme
                            th->apply_cb
                lv_obj_construct
     */

#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/


#endif /* LV_USE_LESSON_DEMO_7_5 */
