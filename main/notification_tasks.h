#ifndef NOTIFICATION_TASKS_H
#define NOTIFICATION_TASKS_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/uart.h>

extern TaskHandle_t notification_by_a;
extern TaskHandle_t notification_by_b;
extern TaskHandle_t notify_shared;

void task_notificator_a(void *);
void task_notificator_b(void *);
void task_bit_ntf_1(void *);
void task_bit_ntf_2(void *);
void task_how_notify_wait_works(void *);





#endif // NOTIFICATION_TASKS_H