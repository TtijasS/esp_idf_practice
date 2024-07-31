#ifndef IRAM_ISRS_H
#define IRAM_ISRS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"

#define UART_BAUD 115200
#define UART_PORT_NUM UART_NUM_1
#define UART_RX_BUFF_SIZE 512
#define UART_TX_BUFF_SIZE 0
#define UART_EVENT_QUEUE_SIZE 16
#define UART_TX_GPIO 43
#define UART_RX_GPIO 44

#define TASK_ISRUART_STACK_SIZE 1024

extern QueueHandle_t uart_event_queue;

// configure uart
extern const uart_config_t uart_config;

// init uart
void uart_init(uart_config_t *);

void task_uart_isr_monitoring(void *);
#endif // IRAM_ISRS_H