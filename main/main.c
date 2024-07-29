#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "driver/uart.h"
#include "simple_tasks.h"
#include "semaphore_tasks.h"

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
     * @brief Semaphorized tasks, same priority.
     */

    // xSemaphoreGive(semaphore_handle);
    // All tasks with same priority, different execution times.
    // All get CPU time.
    // xTaskCreate(&task_sem_a, "Semaphore task A", TASK_SEM_STACK*4, NULL, 5, NULL);
    // xTaskCreate(&task_sem_b, "Semaphore task B", TASK_SEM_STACK*4, NULL, 5, NULL);
    // xTaskCreate(&task_sem_c, "Semaphore task C", TASK_SEM_STACK*4, NULL, 5, NULL);
    // xTaskCreate(&task_sem_d, "Semaphore task D", TASK_SEM_STACK*4, NULL, 5, NULL);

    /*
     * Different priorities. Not all get CPU time
     * A, B, A, C, A, B, A, C,... while D never gets cpu time here, despite being the fastest task
     */
    // xTaskCreate(&task_sem_a, "Semaphore task A", TASK_SEM_STACK*4, NULL, 10, NULL);
    // xTaskCreate(&task_sem_b, "Semaphore task B", TASK_SEM_STACK*4, NULL, 5, NULL);
    // xTaskCreate(&task_sem_c, "Semaphore task C", TASK_SEM_STACK*4, NULL, 2, NULL);
    // xTaskCreate(&task_sem_d, "Semaphore task D", TASK_SEM_STACK*4, NULL, 1, NULL);

    /*
     * You can create multiple identical tasks.
     */
    // xTaskCreate(&task_sem_a, "Semaphore task A", TASK_SEM_STACK*4, NULL, 5, NULL);
    // xTaskCreate(&task_sem_b, "Semaphore task B", TASK_SEM_STACK*4, NULL, 5, NULL);
    // xTaskCreate(&task_sem_c, "Semaphore task C", TASK_SEM_STACK*4, NULL, 5, NULL);
    // xTaskCreate(&task_sem_c, "Semaphore task C pr6", TASK_SEM_STACK*4, NULL, 6, NULL);
    // xTaskCreate(&task_sem_d, "Semaphore task D", TASK_SEM_STACK*4, NULL, 5, NULL);

    /*
     * Same task with arguments, different priority. All get CPU time.
     */
    // uint32_t delay_100 = 100;
    // uint32_t delay_230 = 230;
    // uint32_t delay_520 = 520;
    // uint32_t delay_1230 = 1230;

    // xTaskCreate(&task_sem_x, "A, p5", TASK_SEM_STACK * 4, &delay_100, 5, NULL);
    // xTaskCreate(&task_sem_x, "B, p5", TASK_SEM_STACK * 4, &delay_230, 5, NULL);
    // xTaskCreate(&task_sem_x, "C, p5", TASK_SEM_STACK * 4, &delay_520, 5, NULL);
    // xTaskCreate(&task_sem_x, "D, p5", TASK_SEM_STACK * 4, &delay_1230, 5, NULL);
    
    /*
     * Same task with arguments, different priority. All get CPU time.
     */
    // We use malloc to share the same variable with multiple tasks
    uint32_t *delay_100 = (uint32_t*)malloc(sizeof(uint32_t));
    uint32_t *delay_1000 = (uint32_t*)malloc(sizeof(uint32_t));


    if (delay_100 == NULL || delay_1000 == NULL)
    {
        ESP_LOGE(TAG, "Failed to malloc");
        free(delay_100);
        free(delay_1000);
        return;
    }

    *delay_100 = 100;
    *delay_1000 = 1000;

    semaphore_core_0 = xSemaphoreCreateBinary();
    semaphore_core_1 = xSemaphoreCreateBinary();
    xSemaphoreGive(semaphore_core_0);
    xSemaphoreGive(semaphore_core_1);

    // Tasks running on core 0
    xTaskCreatePinnedToCore(&task_double_core_semaphore, "----A", TASK_SEM_STACK * 4, delay_1000, 5, NULL, 0);
    xTaskCreatePinnedToCore(&task_double_core_semaphore, "----B", TASK_SEM_STACK * 4, delay_1000, 5, NULL, 0);

    // Tasks running on core 1
    xTaskCreatePinnedToCore(&task_double_core_semaphore, "C", TASK_SEM_STACK * 4, delay_100, 10, NULL, 1);
    xTaskCreatePinnedToCore(&task_double_core_semaphore, "D", TASK_SEM_STACK * 4, delay_100, 10, NULL, 1);
    
    // Task without affinity
    xTaskCreatePinnedToCore(&task_double_core_semaphore, "--------E", TASK_SEM_STACK * 5, delay_100, 7, NULL, tskNO_AFFINITY);
}