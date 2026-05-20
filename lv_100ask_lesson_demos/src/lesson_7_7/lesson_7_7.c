/**
 ******************************************************************************
 * @file    lesson_7_7.c
 * @author  百问科技
 * @version V1.0
 * @date    2024-10-08
 * @brief	Lesson 7_7 demo
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

#if LV_USE_LESSON_DEMO_7_7

#include "lesson_7_7.h"

/*********************
 *      DEFINES
 *********************/
#define BUTTOM_SUM 3

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void button_read(lv_indev_t * indev, lv_indev_data_t * data);
static int8_t button_get_pressed_id(void);
static bool button_is_pressed(uint8_t id);


/**********************
 *  STATIC VARIABLES
 **********************/
static uint8_t my_btn_points[BUTTOM_SUM];

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lesson_7_7(void)
{
#if 1 // 7_7

#if 1   // 7-7-1
    // 注入全局钩子
    // 在 lvgl\src\drivers\windows\lv_windows_display.c 的 lv_windows_display_thread_entrypoint 函数中的 GetMessageW 被调用之前添加下面两行代码：
    //extern LRESULT CALLBACK KeyBoardProc(int nCode, WPARAM wParam, LPARAM lParam);
    //SetWindowsHookEx(WH_KEYBOARD_LL, KeyBoardProc, NULL, 0);
#elif 0     // 7-7-2
    /*Register a button input device*/
    lv_indev_t * indev_button = lv_indev_create();
    lv_indev_set_type(indev_button, LV_INDEV_TYPE_BUTTON);
    lv_indev_set_read_cb(indev_button, button_read);

    /*Assign buttons to points on the screen*/
    static const lv_point_t btn_points[BUTTOM_SUM] = {
        {600, 10},   /*Button 0 -> x:600; y:10*/
        {800, 10},   /*Button 1 -> x:800; y:10*/
        {1000, 10},  /*Button 2 -> x:1000; y:10*/
    };
    lv_indev_set_button_points(indev_button, btn_points);
#endif

#endif
}


LRESULT CALLBACK KeyBoardProc(int nCode, WPARAM wParam, LPARAM lParam) {

    if (nCode >= HC_ACTION) {
        KBDLLHOOKSTRUCT kbdStruct = *((KBDLLHOOKSTRUCT *) lParam);
        if (wParam == WM_KEYDOWN) {
            printf("KeyDown: %c\n", kbdStruct.vkCode);
            switch (kbdStruct.vkCode) {
                case '1':
                    my_btn_points[0] = 1;
                    break;
                case '2':
                    my_btn_points[1] = 1;
                    break;
                case '3':
                    my_btn_points[2] = 1;
                    break;
                default:
                    break;
            }
        }
        else if (wParam == WM_KEYUP) {
            printf("KeyUp: %c\n", kbdStruct.vkCode);
            my_btn_points[0] = 0;
            my_btn_points[1] = 0;
            my_btn_points[2] = 0;
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);//调用下一个钩子
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Will be called by the library to read the button*/
static void button_read(lv_indev_t * indev_drv, lv_indev_data_t * data)
{

    static uint8_t last_btn = 0;

    /*Get the pressed button's ID*/
    int8_t btn_act = button_get_pressed_id();

    if(btn_act >= 0) {
        data->state = LV_INDEV_STATE_PRESSED;
        last_btn = btn_act;
    }
    else {
        data->state = LV_INDEV_STATE_RELEASED;
    }

    /*Save the last pressed button's ID*/
    data->btn_id = last_btn;
}

/*Get ID  (0, 1, 2 ..) of the pressed button*/
static int8_t button_get_pressed_id(void)
{
    uint8_t i;

    /*Check to buttons see which is being pressed (assume there are 2 buttons)*/
    for(i = 0; i < BUTTOM_SUM; i++) {
        /*Return the pressed button's ID*/
        if(button_is_pressed(i)) {
            return i;
        }
    }

    /*No button pressed*/
    return -1;
}

/*Test if `id` button is pressed or not*/
static bool button_is_pressed(uint8_t id)
{

    /*Your code comes here*/
    if(id >= BUTTOM_SUM) return false;
    
    return my_btn_points[id];
}


#endif /* LV_USE_LESSON_DEMO_7_7 */
