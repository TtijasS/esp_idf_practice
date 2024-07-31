#ifndef QUEUE_TASKS_H
#define QUEUE_TASKS_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_log.h>

#define QUEUE_LENGTH 10
#define QUEUE_ITEM_SIZE sizeof(uint8_t)

extern QueueHandle_t queue_handle;

void task_main_producer(void *);
void task_producer_2(void *);
void task_consumer(void *);



#endif // QUEUE_TASKS_H