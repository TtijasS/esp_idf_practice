#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "driver/uart.h"
#include "simple_tasks.h"





void app_main()
{
    /**
     * @brief Simple tasks
     */ 
    xTaskCreate(&task_stack_high_watermark, "Hello world task", TASK_HWM_STACK * 4, NULL, 5, NULL);
    int task_param = 5;
    xTaskCreate(&task_signle_param, "Single param task", TASK_SINGLE_PARAM_STACK * 4, &task_param, 5, NULL);
    xTaskCreatePinnedToCore(&task_pinned_to_core, "Pinned task", TASK_PINNED_STACK * 4, NULL, 5, NULL, 0);
    xTaskCreate(&task_self_delete, "Task self delete", TASK_SDELETE_STACK*4, &task_param, 5, NULL);

}