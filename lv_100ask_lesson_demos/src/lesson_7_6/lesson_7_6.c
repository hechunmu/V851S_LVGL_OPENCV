/**
 ******************************************************************************
 * @file    lesson_7_6.c
 * @author  百问科技
 * @version V1.0
 * @date    2024-10-08
 * @brief	Lesson 7_6 demo
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

#if LV_USE_LESSON_DEMO_7_6

#include "lesson_7_6.h"

// 用Windows PC模拟器键盘或鼠标需要包含此头文件
#include "lvgl/src/drivers/windows/lv_windows_context.h"

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
void lesson_7_6(void)
{
#if 1 // 7_6

#if 1 // 7_6_1
    lv_windows_acquire_encoder_indev(lv_display_get_default());  // 鼠标
    lv_windows_acquire_keypad_indev(lv_display_get_default());   // 键盘


    HWND window_handle = lv_windows_get_display_window_handle(lv_display_get_default());
    if(!window_handle) {
        return NULL;
    }

    lv_windows_window_context_t * context = lv_windows_get_window_context(window_handle);
    if(!context) {
        return NULL;
    }

    // 创建一个组，稍后将需要使用键盘或编码器或按钮控制的部件(对象)添加进去，并且将输入设备和组关联
    // 如果将这个组设置为默认组，那么对于那些在创建时会添加到默认组的部件(对象)就可以省略 lv_group_add_obj()
    lv_group_t * g = lv_group_create();

    // 将上面创建的组设置为默认组
    // 如果稍后创建的部件(对象)，使用默认组那必须要在其创建之前设置好默认组，否则不生效
    lv_group_set_default(g);

    if(context->encoder.indev) {
       lv_indev_set_group(context->encoder.indev, g);
    }

    if(context->keypad.indev) {
       lv_indev_set_group(context->keypad.indev, lv_group_get_default());
    }

    //lv_obj_t * btn1 = lv_button_create(lv_screen_active()); // 一出生就加入到默认组里面

    /// 
    static const char * btnm_map[] = {"1", "2", "3", "4", "5", "\n",
                                  "6", "7", "8", "9", "0", "\n",
                                  "Action1", "Action2", ""
                                 };

    lv_obj_t * btnm1 = lv_buttonmatrix_create(lv_screen_active());
    lv_buttonmatrix_set_map(btnm1, btnm_map);
    lv_buttonmatrix_set_button_width(btnm1, 10, 2);        /*Make "Action1" twice as wide as "Action2"*/
    lv_buttonmatrix_set_button_ctrl(btnm1, 10, LV_BUTTONMATRIX_CTRL_CHECKABLE);
    lv_buttonmatrix_set_button_ctrl(btnm1, 11, LV_BUTTONMATRIX_CTRL_CHECKED);
    lv_obj_align(btnm1, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * btn1 = lv_button_create(lv_screen_active());

    lv_group_add_obj(g, btnm1);
    lv_group_add_obj(lv_group_get_default(), btn1);
#elif 1 // 7_6_3
    lv_init
    lv_group_init
    lv_windows_platform_init
    	lv_group_create
    	lv_group_set_default
    
    ////////////////////////////
    lv_windows_acquire_encoder_indev
        lv_indev_create
            lv_timer_create
        lv_indev_set_read_cb -> lv_windows_encoder_device_window_message_handler
        lv_indev_set_group
        
    ////////////////////////////
    lv_xxx_create
        lv_obj_class_create_obj
        lv_obj_class_init_obj
            lv_group_add_obj // if has def_group and lv_obj_is_group_def
#endif

#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/


#endif /* LV_USE_LESSON_DEMO_7_6 */
