#ifndef APP_GPIO_H
#define APP_GPIO_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    APP_GPIO_VALVE1 = 0,
    APP_GPIO_VALVE2,
    APP_GPIO_VALVE3,
    APP_GPIO_MAGNET,
    APP_GPIO_STEPPER,
    APP_GPIO_CAMERA,
    APP_GPIO_OUTPUT_COUNT
} app_gpio_id_t;

typedef void (*app_gpio_button_handler_t)(uint32_t id, bool enable, void *user_data);

void app_gpio_init(void);
void app_gpio_cleanup(void);
void app_gpio_apply_output(uint32_t id, bool enable, bool sync_ui);
void app_gpio_create_button_panel(app_gpio_button_handler_t handler, void *user_data);

#endif