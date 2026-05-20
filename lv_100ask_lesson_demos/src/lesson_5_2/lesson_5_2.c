/**
 ******************************************************************************
 * @file    lesson_5_2.c
 * @author  百问科技
 * @version V1.0
 * @date    2024-9-25
 * @brief	Lesson 5_2 demo
 ******************************************************************************
 * Change Logs:
 * Date           Author          Notes
 * 2024-9-25     zhouyuebiao     First version
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

#if LV_USE_LESSON_DEMO_5_2

#include "lesson_5_2.h"

/*********************
 *      DEFINES
 *********************/
#define IMAGES_PATH  		"D:/100ask/images/"
#define IMAGES_FILE_BMP  	"example_32bit.bmp"
#define IMAGES_FILE_JPG  	"flower.jpg"
#define IMAGES_FILE_PNG  	"100ask_logo.png"
#define IMAGES_FILE_GIF 	"duolaameng.gif"


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
void lesson_5_2(void)
{
#if 0 // 5_2_1
	lv_obj_t * img;

	lv_obj_set_flex_flow(lv_screen_active(), LV_FLEX_FLOW_ROW);

	img = lv_image_create(lv_screen_active());
    lv_image_set_src(img, IMAGES_PATH IMAGES_FILE_BMP);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

	img = lv_image_create(lv_screen_active());
    lv_image_set_src(img, IMAGES_PATH IMAGES_FILE_JPG);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

	img = lv_image_create(lv_screen_active());
    lv_image_set_src(img, IMAGES_PATH IMAGES_FILE_PNG);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

	img = lv_gif_create(lv_screen_active());
    lv_gif_set_src(img, IMAGES_PATH IMAGES_FILE_GIF);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

#endif

}

/**********************
 *   STATIC FUNCTIONS
 **********************/


#endif /* LV_USE_LESSON_DEMO_5_2 */
