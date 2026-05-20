/**
 * @file lv_port_indev.c
 *
 */

/*Copy this file as "lv_port_indev.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_indev.h"

// 100ask
#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <string.h>  
#include <sys/types.h>  
#include <fcntl.h>  
#include <errno.h>  
#include <time.h>  
#include <linux/input.h>
#include <pthread.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void touchpad_init(void);
static void touchpad_read(lv_indev_t * indev, lv_indev_data_t * data);
static bool touchpad_is_pressed(void);
static void touchpad_get_xy(int32_t * x, int32_t * y);

static void mouse_init(void);
static void mouse_read(lv_indev_t * indev, lv_indev_data_t * data);
static bool mouse_is_pressed(void);
static void mouse_get_xy(int32_t * x, int32_t * y);

static void keypad_init(void);
static void keypad_read(lv_indev_t * indev, lv_indev_data_t * data);
static uint32_t keypad_get_key(void);

static void encoder_init(void);
static void encoder_read(lv_indev_t * indev, lv_indev_data_t * data);
static void encoder_handler(void);

// 100ask
static int8_t encoder_get_pressed_id(void);
static int8_t keypad_get_pressed_id(void);

static void button_init(void);
static void button_read(lv_indev_t * indev, lv_indev_data_t * data);
static int8_t button_get_pressed_id(void);
static bool button_is_pressed(uint8_t id);

/**********************
 *  STATIC VARIABLES
 **********************/
lv_indev_t * indev_touchpad;
lv_indev_t * indev_mouse;
lv_indev_t * indev_keypad;
lv_indev_t * indev_encoder;
lv_indev_t * indev_button;

static int32_t encoder_diff;
static lv_indev_state_t encoder_state;

/// 100ask
static struct input_event t;

#define BUTTOM_SUM                  3
#define ENCODER_BUTTOM_SUM          3
#define KEYPAD_ENCODER_BUTTOM_SUM   5

static uint8_t g_my_btn_points[BUTTOM_SUM];
static uint8_t g_my_encoder_btn_points[ENCODER_BUTTOM_SUM];
static uint8_t g_my_keypad_btn_points[KEYPAD_ENCODER_BUTTOM_SUM];


/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
static void * input_read_thread_function(void *arg)
{
    int keys_fd;

    keys_fd = open("/dev/input/event6", O_RDONLY | O_NOCTTY | O_CLOEXEC);
	if (keys_fd <= 0)
	{
		printf("open /dev/input/event6 device error!\n");
		exit(EXIT_FAILURE);
	}

    if(fcntl(keys_fd, F_SETFL, O_NONBLOCK) < 0)
	{
        perror("Failed to fcntl device");
        exit(EXIT_FAILURE);
    }

    while (1)
	{
		if (read(keys_fd, &t, sizeof (t)) == sizeof (t))
		{
			if (t.type == EV_KEY)
            {
                //printf("key %d %s\n", t.code, (t.value) ? "Pressed" : "Released");
                // 键码表：https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/include/uapi/linux/input-event-codes.h
                // 或 lvgl_100ask_course_v9/part3/02_codes/lesson_4-2/input-event-codes.h
                if (t.value == 1)
                {
                    switch (t.code) {
                        case KEY_1:
                            g_my_btn_points[0] = 1;
                            break;
                        case KEY_2:
                            g_my_btn_points[1] = 1;
                            break;
                        case KEY_3:
                            g_my_btn_points[2] = 1;
                            break;
                        case KEY_4:
                            g_my_encoder_btn_points[0] = 1;
                            break;
                        case KEY_5:
                            g_my_encoder_btn_points[1] = 1;
                            break;
                        case KEY_6:
                            g_my_encoder_btn_points[2] = 1;
                            break;  
                        case KEY_Q:
                            g_my_keypad_btn_points[0] = 1;
                            break;
                        case KEY_W:
                            g_my_keypad_btn_points[1] = 1;
                            break;
                        case KEY_E:
                            g_my_keypad_btn_points[2] = 1;
                            break;
                        case KEY_R:
                            g_my_keypad_btn_points[3] = 1;
                            break;
                        case KEY_T:
                            g_my_keypad_btn_points[4] = 1;
                            break;
                             
                        default:
                            break;
                    }
                }
                else if (t.value == 0)
                {
                    g_my_btn_points[0] = 0;
                    g_my_btn_points[1] = 0;
                    g_my_btn_points[2] = 0;

                    g_my_encoder_btn_points[0] = 0;
                    g_my_encoder_btn_points[1] = 0;
                    g_my_encoder_btn_points[2] = 0;

                    g_my_keypad_btn_points[0] = 0;
                    g_my_keypad_btn_points[1] = 0;
                    g_my_keypad_btn_points[2] = 0;
                    g_my_keypad_btn_points[3] = 0;
                    g_my_keypad_btn_points[4] = 0;
                }
            }
		}
        usleep(30000);
	}

    close(keys_fd);
}


void lv_port_indev_init(void)
{
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, input_read_thread_function, NULL);

    /**
     * Here you will find example implementation of input devices supported by LittelvGL:
     *  - Touchpad
     *  - Mouse (with cursor support)
     *  - Keypad (supports GUI usage only with key)
     *  - Encoder (supports GUI usage only with: left, right, push)
     *  - Button (external buttons to press points on the screen)
     *
     *  The `..._read()` function are only examples.
     *  You should shape them according to your hardware
     */
#if 0
    /*------------------
     * Touchpad
     * -----------------*/

    /*Initialize your touchpad if you have*/
    touchpad_init();

    /*Register a touchpad input device*/
    indev_touchpad = lv_indev_create();
    lv_indev_set_type(indev_touchpad, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev_touchpad, touchpad_read);

    /*------------------
     * Mouse
     * -----------------*/

    /*Initialize your mouse if you have*/
    mouse_init();

    /*Register a mouse input device*/
    indev_mouse = lv_indev_create();
    lv_indev_set_type(indev_mouse, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev_mouse, mouse_read);

    /*Set cursor. For simplicity set a HOME symbol now.*/
    lv_obj_t * mouse_cursor = lv_image_create(lv_screen_active());
    lv_image_set_src(mouse_cursor, LV_SYMBOL_HOME);
    lv_indev_set_cursor(indev_mouse, mouse_cursor);

    /*------------------
     * Keypad
     * -----------------*/

    /*Initialize your keypad or keyboard if you have*/
    keypad_init();

    /*Register a keypad input device*/
    indev_keypad = lv_indev_create();
    lv_indev_set_type(indev_keypad, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(indev_keypad, keypad_read);

    /*Later you should create group(s) with `lv_group_t * group = lv_group_create()`,
     *add objects to the group with `lv_group_add_obj(group, obj)`
     *and assign this input device to group to navigate in it:
     *`lv_indev_set_group(indev_keypad, group);`*/

    /*------------------
     * Encoder
     * -----------------*/

    /*Initialize your encoder if you have*/
    encoder_init();

    /*Register a encoder input device*/
    indev_encoder = lv_indev_create();
    lv_indev_set_type(indev_encoder, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_read_cb(indev_encoder, encoder_read);

    /*Later you should create group(s) with `lv_group_t * group = lv_group_create()`,
     *add objects to the group with `lv_group_add_obj(group, obj)`
     *and assign this input device to group to navigate in it:
     *`lv_indev_set_group(indev_encoder, group);`*/

    /*------------------
     * Button
     * -----------------*/

    /*Initialize your button if you have*/
    button_init();

    /*Register a button input device*/
    indev_button = lv_indev_create();
    lv_indev_set_type(indev_button, LV_INDEV_TYPE_BUTTON);
    lv_indev_set_read_cb(indev_button, button_read);

    /*Assign buttons to points on the screen*/
    static const lv_point_t btn_points[2] = {
        {10, 10},   /*Button 0 -> x:10; y:10*/
        {40, 100},  /*Button 1 -> x:40; y:100*/
    };
    lv_indev_set_button_points(indev_button, btn_points);
#endif

#if 1  // 4-3
    /*Register a button input device*/
    indev_button = lv_indev_create();
    lv_indev_set_type(indev_button, LV_INDEV_TYPE_BUTTON);
    lv_indev_set_read_cb(indev_button, button_read);

    /*Assign buttons to points on the screen*/
    static const lv_point_t btn_points[BUTTOM_SUM] = {
        {100, 20},  /*Button 0 -> x:100; y:20*/
		{220, 20},  /*Button 1 -> x:220; y:20*/
		{380, 20},  /*Button 1 -> x:380; y:20*/
    };
    lv_indev_set_button_points(indev_button, btn_points);
#endif

    lv_group_t * group = lv_group_create();
    lv_group_set_default(group);

    lv_group_t * group2 = lv_group_create();
    //lv_group_set_default(group2);

#if 1 // 4-4
    /*------------------
     * Encoder
     * -----------------*/

    /*Initialize your encoder if you have*/
    encoder_init();

    /*Register a encoder input device*/
    indev_encoder = lv_indev_create();
    lv_indev_set_type(indev_encoder, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_read_cb(indev_encoder, encoder_read);

    lv_indev_set_group(indev_encoder, group);
#endif

#if 1 // 4-5
    /*------------------
     * Keypad
     * -----------------*/

    /*Initialize your keypad or keyboard if you have*/
    keypad_init();

    /*Register a keypad input device*/
    indev_keypad = lv_indev_create();
    lv_indev_set_type(indev_keypad, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(indev_keypad, keypad_read);

    lv_indev_set_group(indev_keypad, group);
    //lv_indev_set_group(indev_keypad, group2);
#endif

}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*------------------
 * Touchpad
 * -----------------*/
int touch_fd;
/*Initialize your touchpad*/
static void touchpad_init(void)
{
    touch_fd = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
    if(touch_fd < 0) {
        perror("open touch fd failed");
        return;
    }
}


/*Will be called by the library to read the touchpad*/
static void touchpad_read(lv_indev_t * indev_drv, lv_indev_data_t * data)
{
    static int32_t last_x = 0;
    static int32_t last_y = 0;

    /*Save the pressed coordinates and the state*/
    if(touchpad_is_pressed()) {
        touchpad_get_xy(&last_x, &last_y);
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else {
        data->state = LV_INDEV_STATE_RELEASED;
    }

    /*Set the last pressed coordinates*/
    data->point.x = last_x;
    data->point.y = last_y;
}

/*Return true is the touchpad is pressed*/
static bool touchpad_is_pressed(void)
{
    /*Your code comes here*/

    return false;
}

/*Get the x and y coordinates if the touchpad is pressed*/
static void touchpad_get_xy(int32_t * x, int32_t * y)
{
    /*Your code comes here*/

    (*x) = 0;
    (*y) = 0;
}

/*------------------
 * Mouse
 * -----------------*/

/*Initialize your mouse*/
static void mouse_init(void)
{
    /*Your code comes here*/
}

/*Will be called by the library to read the mouse*/
static void mouse_read(lv_indev_t * indev_drv, lv_indev_data_t * data)
{
    /*Get the current x and y coordinates*/
    mouse_get_xy(&data->point.x, &data->point.y);

    /*Get whether the mouse button is pressed or released*/
    if(mouse_is_pressed()) {
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

/*Return true is the mouse button is pressed*/
static bool mouse_is_pressed(void)
{
    /*Your code comes here*/

    return false;
}

/*Get the x and y coordinates if the mouse is pressed*/
static void mouse_get_xy(int32_t * x, int32_t * y)
{
    /*Your code comes here*/

    (*x) = 0;
    (*y) = 0;
}

/*------------------
 * Keypad
 * -----------------*/

/*Initialize your keypad*/
static void keypad_init(void)
{
    /*Your code comes here*/
}

/*Will be called by the library to read the mouse*/
static void keypad_read(lv_indev_t * indev_drv, lv_indev_data_t * data)
{
    static uint32_t last_key = 0;

    /*Get the current x and y coordinates*/
    //mouse_get_xy(&data->point.x, &data->point.y);

    /*Get whether the a key is pressed and save the pressed key*/
    uint32_t act_key = keypad_get_pressed_id();
    if(act_key != -1) {
        data->state = LV_INDEV_STATE_PRESSED;

        /*Translate the keys to LVGL control characters according to your key definitions*/
        switch(act_key) {
            case 0: // Q
                act_key = LV_KEY_NEXT;
                break;
            case 1: // W
                act_key = LV_KEY_PREV;
                break;
            case 2: // E
                act_key = LV_KEY_LEFT;
                break;
            case 3: // R
                act_key = LV_KEY_RIGHT;
                break;
            case 4: // T
                act_key = LV_KEY_ENTER;
                break;
        }

        last_key = act_key;
    }
    else {
        data->state = LV_INDEV_STATE_RELEASED;
    }

    data->key = last_key;
}

/*Get the currently being pressed key.  0 if no key is pressed*/
static uint32_t keypad_get_key(void)
{
    /*Your code comes here*/

    return 0;
}

/*------------------
 * Encoder
 * -----------------*/

/*Initialize your encoder*/
static void encoder_init(void)
{
    /*Your code comes here*/
}

/*Will be called by the library to read the encoder*/
static void encoder_read(lv_indev_t * indev_drv, lv_indev_data_t * data)
{
    //int8_t btn_act = button_get_pressed_id();
    int8_t btn_act = encoder_get_pressed_id();

    switch (btn_act)
    {
    case 0:
        encoder_diff -= 1;
        encoder_state = LV_INDEV_STATE_RELEASED;
        break;
    case 1:
        encoder_diff = 0;
        encoder_state = LV_INDEV_STATE_PRESSED;
        break;
    case 2:
        encoder_diff += 1;
        encoder_state = LV_INDEV_STATE_RELEASED;
        break;
    
    default:
        encoder_diff = 0;
        encoder_state = LV_INDEV_STATE_RELEASED;
        break;
    }

    data->enc_diff = encoder_diff;
    data->state = encoder_state;
}

/*Call this function in an interrupt to process encoder events (turn, press)*/
static void encoder_handler(void)
{
    /*Your code comes here*/

    encoder_diff += 1;
    encoder_state = LV_INDEV_STATE_RELEASED;
}

/*------------------
 * Button
 * -----------------*/

/*Initialize your buttons*/
static void button_init(void)
{
    /*Your code comes here*/
}

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
    bool ret = false;

    /*Your code comes here*/

    if(id >= BUTTOM_SUM) return false;
    
    ret = g_my_btn_points[id];
    g_my_btn_points[id] = 0;

    return ret;

    //return false;
}

// 100ask
static int8_t encoder_get_pressed_id(void)
{
    uint8_t i;

    /*Check to buttons see which is being pressed (assume there are 2 buttons)*/
    for(i = 0; i < ENCODER_BUTTOM_SUM; i++) {
        /*Return the pressed button's ID*/
        if(g_my_encoder_btn_points[i]) {
            g_my_encoder_btn_points[i] = 0;
            return i;
        }
    }

    /*No button pressed*/
    return -1;
}

static int8_t keypad_get_pressed_id(void)
{
    uint8_t i;

    /*Check to buttons see which is being pressed (assume there are 2 buttons)*/
    for(i = 0; i < KEYPAD_ENCODER_BUTTOM_SUM; i++) {
        /*Return the pressed button's ID*/
        if(g_my_keypad_btn_points[i]) {
            g_my_keypad_btn_points[i] = 0;
            return i;
        }
    }

    /*No button pressed*/
    return -1;
}



#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
