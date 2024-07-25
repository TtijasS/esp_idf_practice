#ifndef SIMPLE_TASKS_H
#define SIMPLE_TASKS_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "driver/uart.h"

#define TASK_HWM_STACK 512
#define TASK_SINGLE_PARAM_STACK 512
#define TASK_PINNED_STACK 512
#define TASK_SDELETE_STACK 512

void task_stack_high_watermark(void *);
void task_signle_param(void *);
void task_pinned_to_core(void *);
void task_self_delete(void *);


#endif // SIMPLE_TASKS_H