/**
 ******************************************************************************
 * @file    lesson_5_1.c
 * @author  百问科技
 * @version V1.0
 * @date    2024-9-24
 * @brief	Lesson 5_1 demo
 ******************************************************************************
 * Change Logs:
 * Date           Author          Notes
 * 2024-9-24     zhouyuebiao     First version
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

#if LV_USE_LESSON_DEMO_5_1

#include "lesson_5_1.h"


#include <stdio.h>   // 通过C语言接口读写文件头文件包含
#include <dirent.h>  // 通过C语言接口读目录头文件包含

/*********************
 *      DEFINES
 *********************/
// 要打开的文件
#define FILE_NAME	"lv_fs_test.txt"
// 要读取的目录
#define DIR_PATH	"D:/100ask/"

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
/* 通过LVGL文件系统接口统一不同的文件系统并读取文件 */
//static void lv_fs_read_dir(char * fn);

/* 通过LVGL文件系统接口统一不同的文件系统并读取目录内容 */
static void lv_fs_read_file(char * path);

/* 通过c语言接口读取文件 */
static void c_read_file(char * fn);

/* 通过c语言接口读取目录内容 */
static void c_read_dir(char *path);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lesson_5_1(void)
{
#if 1 // 5_1

    /* C语言接口 */
    // 读取文件（C语言）
	//c_read_file(FILE_NAME);

	// 读取目录内容（C语言）
	//c_read_dir(DIR_PATH);

    /* LVGL接口 */
	// 读取文件（LVGL）
	//lv_fs_read_file(FILE_NAME);

	// 读取目录内容（LVGL）
	//lv_fs_read_dir(DIR_PATH);

#endif

}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/* 通过LVGL文件系统接口统一不同的文件系统并读取文件 */
static void lv_fs_read_file(char * fn)
{
	lv_fs_file_t f;
	lv_fs_res_t res;

	// 打开文件有两个模式： LV_FS_MODE_RD(只读) 和 LV_FS_MODE_WR(写)
	res = lv_fs_open(&f, fn, LV_FS_MODE_RD);
	// 如果一切正常会返回 LV_FS_RES_OK ，其他错误代码请看 lv_fs.h 中的 lv_fs_res_t 定义
	if(res != LV_FS_RES_OK) {
		LV_LOG_USER("Open error! Error code: %d", res);
		return;
	}

	/* 每次实际读取到的数据大小(byte) */
	uint32_t read_num;
	/* 数据缓冲区 */
	uint8_t buf[8];

	/* 读取整个文件并打印内容 */
	while (1) {
		res = lv_fs_read(&f, buf, 7, &read_num);
		if(res != LV_FS_RES_OK) {
			LV_LOG_USER("Read error! Error code: %d", res);
			break;
		}

		/* 将读取到数据打印出来 */
		printf("%s", buf);

		if (read_num != 7)	break;
	}

	lv_fs_close(&f);

}


#if 0
/* 通过LVGL文件系统接口统一不同的文件系统并读取目录内容 */
static void lv_fs_read_dir(char * path)
{
	lv_fs_dir_t dir;
	lv_fs_res_t res;

	res = lv_fs_dir_open(&dir, path);
	if(res != LV_FS_RES_OK){
		LV_LOG_USER("Open DIR error! Error code: %d", res);
		return;
	}

	char fn[128];	// 缓冲区
	while(1) {
		res = lv_fs_dir_read(&dir, fn, 128);
		if(res != LV_FS_RES_OK) {
			LV_LOG_USER("\nRead DIR error! Error code: %d", res);
			break;
		}

		/* 如果没有更多文件可以读取时 fn 就为空 */
		if(strlen(fn) == 0) {
			LV_LOG_USER("\n\nFn is empty, not more files to read.");
			break;
		}

		printf("%s\n", fn);
	}

	lv_fs_dir_close(&dir);

}
#endif

////////////////////////////////////////////////////////////
/* 通过c语言接口读取文件 */
static void c_read_file(char * fn)
{
	FILE *fp = NULL;

	fp = fopen(fn, "r");
	// 如果一切正常会返回 LV_FS_RES_OK ，其他错误代码请看 lv_fs.h 中的 lv_fs_res_t 定义
	if(fp == NULL) {
        printf("Open file error!\n");
		return;
	}

	/* 每次实际读取到的数据大小(byte) */
	uint32_t read_num;
	/* 数据缓冲区 */
	uint8_t buf[8];

	/* 读取整个文件并打印内容 */
	while (1) {
		if(fgets(buf, 8, fp) == NULL) {
			printf("\n\nRead file end!\n");
			break;
		}

		/* 将读取到数据打印出来 */
		printf("%s", buf);
	}

	fclose(fp);

}

/* 通过c语言接口读取目录内容 */
static void c_read_dir(char *path)
{
	DIR *dir = opendir(path);//打开目录文件
	struct dirent *entry;

	while((entry = readdir(dir))!=0)
	{
		printf("%s\n", entry->d_name);
	}

    closedir(dir);
}


#endif /* LV_USE_LESSON_DEMO_5_1 */
