#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "driver/uart.h"

#define TASK_HWM_STACK 512
#define TASK_SINGLE_PARAM_STACK 512
#define TASK_PINNED_STACK 512
#define TASK_SDELETE_STACK 512
#define TASK_

/**
 * @brief This is how you get task high watermark
 *
 * @param pvParameters
 */
void task_stack_high_watermark(void *pvParameters)
{
    static const char *TAG_HWM_TASK = "Task HWM";
    while (1)
    {
        ESP_LOGI(TAG_HWM_TASK, "Hello world!");
        UBaseType_t stack_hwm = uxTaskGetStackHighWaterMark(NULL);
        ESP_LOGI(TAG_HWM_TASK, "Free stack size: %u W", stack_hwm);
        ESP_LOGI(TAG_HWM_TASK, "Stack in use: %u W", TASK_HWM_STACK - stack_hwm);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void task_signle_param(void *pvParameters)
{
    static const char *TAG_PARAM_TASK = "Task SP";
    int parameter = *(int *)pvParameters;
    while (1)
    {
        ESP_LOGI(TAG_PARAM_TASK, "Task received parameter: %d", parameter);
        UBaseType_t stack_hwm = uxTaskGetStackHighWaterMark(NULL);
        ESP_LOGI(TAG_PARAM_TASK, "Free stack size: %u W", stack_hwm);
        ESP_LOGI(TAG_PARAM_TASK, "Stack in use: %u W", TASK_HWM_STACK - stack_hwm);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void task_pinned_to_core(void *pvParameters)
{
    const char *TAG_PINNED_TASK = "Task PINNED";
    while (1)
    {
        ESP_LOGI(TAG_PINNED_TASK, "Task pinned to core %d", xPortGetCoreID());
        UBaseType_t stack_hwm = uxTaskGetStackHighWaterMark(NULL);
        ESP_LOGI(TAG_PINNED_TASK, "Free stack size: %u W", stack_hwm);
        ESP_LOGI(TAG_PINNED_TASK, "Stack in use: %u W", TASK_HWM_STACK - stack_hwm);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void task_self_delete(void *pvParameters)
{
    const char* TAG_SDELETE_TASK = "Task SDELETE";
    int param = *(int *)pvParameters;

    ESP_LOGI(TAG_SDELETE_TASK, "Received param %d", param);

    for (int i = 0; i < param; i++)
    {
        ESP_LOGI(TAG_SDELETE_TASK, "Task ran %d times", i);
        if (i == (param - 1)) // Check if it's the last iteration
            ESP_LOGI(TAG_SDELETE_TASK, "Task finished and will self delete");
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay to simulate task doing work
    }

    UBaseType_t stack_hwm = uxTaskGetStackHighWaterMark(NULL);
    ESP_LOGI(TAG_SDELETE_TASK, "Free stack size: %u W", stack_hwm);
    ESP_LOGI(TAG_SDELETE_TASK, "Stack in use: %u W", TASK_HWM_STACK - stack_hwm);

    vTaskDelete(NULL);
}

void task_main_handler(void *pvParameters)
{
    static const char* TAG_MAIN_HANDLER_TASK = "Task Main Handler";
    int *parameter = (int *)pvParameters;

    while (1)
    {

    }
}

void app_main()
{
    /**
     * @brief Simple tasks
     */ 
    // xTaskCreate(&task_stack_high_watermark, "Hello world task", TASK_HWM_STACK * 4, NULL, 5, NULL);
    // int task_param = 5;
    // xTaskCreate(&task_signle_param, "Single param task", TASK_SINGLE_PARAM_STACK * 4, &task_param, 5, NULL);
    // xTaskCreatePinnedToCore(&task_pinned_to_core, "Pinned task", TASK_PINNED_STACK * 4, NULL, 5, NULL, 0);
    // xTaskCreate(&task_self_delete, "Task self delete", TASK_SDELETE_STACK*4, &task_param, 5, NULL);

    /**
     * @brief Interacting tasks
     */

}