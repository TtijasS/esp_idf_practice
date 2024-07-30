#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "driver/uart.h"
#include "simple_tasks.h"
#include "semaphore_tasks.h"
#include "queue_tasks.h"

static const char* TAG = "Tasks";

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
     * @brief Semaphore tasks with different priorities and different core id affinities.
     */
    // We use malloc to share the same value with different tasks and across the cores.
    // uint32_t *delay_100 = (uint32_t*)malloc(sizeof(uint32_t));
    // uint32_t *delay_1000 = (uint32_t*)malloc(sizeof(uint32_t));


    // if (delay_100 == NULL || delay_1000 == NULL)
    // {
    //     ESP_LOGE(TAG, "Failed to malloc");
    //     free(delay_100);
    //     free(delay_1000);
    //     return;
    // }

    // *delay_100 = 100;
    // *delay_1000 = 1000;

    // semaphore_core_0 = xSemaphoreCreateBinary();
    // semaphore_core_1 = xSemaphoreCreateBinary();

    // // Tasks running on core 0
    // xTaskCreatePinnedToCore(&task_double_core_semaphore, "----A", TASK_SEM_STACK * 4, delay_1000, 5, NULL, 0);
    // xTaskCreatePinnedToCore(&task_double_core_semaphore, "----B", TASK_SEM_STACK * 4, delay_1000, 5, NULL, 0);

    // // Tasks running on core 1
    // xTaskCreatePinnedToCore(&task_double_core_semaphore, "C", TASK_SEM_STACK * 4, delay_100, 10, NULL, 1);
    // xTaskCreatePinnedToCore(&task_double_core_semaphore, "D", TASK_SEM_STACK * 4, delay_100, 10, NULL, 1);
    
    // // Task without affinity
    // // If E has the highest priority it will use both cores, otherwise it will take CPU time from lower priority tasks
    // xTaskCreatePinnedToCore(&task_double_core_semaphore, "--------E", TASK_SEM_STACK * 5, delay_100, 11, NULL, tskNO_AFFINITY);

    // xSemaphoreGive(semaphore_core_0);
    // xSemaphoreGive(semaphore_core_1);

    /**
     * @brief Queue tasks
     * 
     * Tasks using queue handle
     */

    // queue_handle = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);

    // if (queue_handle == NULL)
    // {
    //     ESP_LOGE("Queue handle", "Failed to init queue");
    //     return;
    // }

    // xTaskCreate(&task_main_producer, "Producer", TASK_QUEUE_STACK*4, NULL, 10, NULL);
    // xTaskCreate(&task_producer_2, "Producer 2", TASK_QUEUE_STACK*4, NULL, 10, NULL);
    // xTaskCreate(&task_consumer, "Consumer", TASK_QUEUE_STACK*4, NULL, 10, NULL);
}