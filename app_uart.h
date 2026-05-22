#ifndef APP_UART_H
#define APP_UART_H

#include <signal.h>
#include <stddef.h>
#include <stdint.h>

typedef void (*app_uart_frame_handler_t)(const uint8_t frame[4], void *user_data);

void app_uart_init(volatile sig_atomic_t *running_flag,
                   app_uart_frame_handler_t frame_handler,
                   void *user_data);
void app_uart_cleanup(void);
int app_uart_send_frame(const uint8_t *data, size_t len);

#endif