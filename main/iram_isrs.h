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
#define UART_NUM UART_NUM_0
#define UART_RX_BUFF_SIZE 1024
#define UART_TX_BUFF_SIZE 0
#define UART_EVENT_QUEUE_SIZE 32 /*!< Number of UART ISR events queued*/
#define UART_PAT_POS_QUEUE_SIZE 8 /*!< Number of queued pattern index positions*/
#define U0TXD 43
#define U0RXD 44
#define U1TXD 17
#define U1RXD 18

#define TASK_ISRUART_STACK_SIZE 1024
#define UART_PATTERN_SIZE 8
extern QueueHandle_t uart_event_queue_handle;

// configure uart
extern uart_config_t uart_config;
extern uart_config_t uart_config_1;

// init uart
void uart_init(uart_config_t *uart_config, uart_port_t port_num, int gpio_tx, int gpio_rx, int tx_buff_size, int rx_buff_size);
void uart_init_with_isr_queue(uart_config_t *uart_config, uart_port_t port_num, int gpio_tx, int gpio_rx, int tx_buff_size, int rx_buff_size, QueueHandle_t *isr_queue_handle, int isr_queue_size, int intr_alloc_flags);

int uart_encapsulation_handler(int *pattern_positions, int *pattern_counter);

void task_uart_isr_monitoring(void *);
#endif // IRAM_ISRS_H