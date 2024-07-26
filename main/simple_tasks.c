#include "simple_tasks.h"

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


/**
 * @brief This task takes a single param and uses it
 * 
 * @param pvParameters 
 */
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

/**
 * @brief This task is pinned to specific core
 * 
 * @param pvParameters 
 */
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

/**
 * @brief This task takes a single param and uses it to loop for n-times, then deletes itself.
 * 
 * @param pvParameters 
 */
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