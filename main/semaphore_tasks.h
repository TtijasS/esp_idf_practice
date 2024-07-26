#ifndef SEMAPHORE_TASKS_H
#define SEMAPHORE_TASKS_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "esp_log.h"
#include "driver/uart.h"

#define TASK_SEM_STACK 512


extern SemaphoreHandle_t semaphore_handle;

void task_sem_a(void *);
void task_sem_b(void *);
void task_sem_c(void *);
void task_sem_d(void *);
void task_sem_x(void *);


#endif // SEMAPHORE_TASKS_H