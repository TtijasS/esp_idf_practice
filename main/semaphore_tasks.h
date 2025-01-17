#ifndef SEMAPHORE_TASKS_H
#define SEMAPHORE_TASKS_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "esp_log.h"


extern SemaphoreHandle_t semaphore_core_0;
extern SemaphoreHandle_t semaphore_core_1;

void task_sem_a(void *);
void task_sem_b(void *);
void task_sem_c(void *);
void task_sem_d(void *);
void task_double_core_semaphore(void *);


#endif // SEMAPHORE_TASKS_H