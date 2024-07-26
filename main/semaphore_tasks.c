#include "semaphore_tasks.h"

SemaphoreHandle_t semaphore_handle;

void task_sem_a(void *pvParameters)
{
	static const char *TAG_TASK_A = "Task A";

	while (1)
	{
		if (xSemaphoreTake(semaphore_handle, portMAX_DELAY) == pdTRUE)
		{
			ESP_LOGI(TAG_TASK_A, "Taken by A (delay 1000 ms)");
			vTaskDelay(pdMS_TO_TICKS(1000));
			ESP_LOGI(TAG_TASK_A, "Given by A (delay 1000 ms)");
			xSemaphoreGive(semaphore_handle);
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
	}
}

void task_sem_b(void *pvParameters)
{
	static const char *TAG_TASK_B = "Task B";

	while (1)
	{
		if (xSemaphoreTake(semaphore_handle, portMAX_DELAY) == pdTRUE)
		{
			ESP_LOGI(TAG_TASK_B, "-----Taken by B (delay 1250 ms)");
			vTaskDelay(pdMS_TO_TICKS(1250));
			ESP_LOGI(TAG_TASK_B, "-----Given by B (delay 1250 ms)");
			xSemaphoreGive(semaphore_handle);
			vTaskDelay(pdMS_TO_TICKS(1250));
		}
	}
}

void task_sem_c(void *pvParameters)
{
	static const char *TAG_TASK_C = "Task C";

	while (1)
	{
		if (xSemaphoreTake(semaphore_handle, portMAX_DELAY) == pdTRUE)
		{
			ESP_LOGI(TAG_TASK_C, "----------Taken by C (delay 2100 ms)");
			vTaskDelay(pdMS_TO_TICKS(2100));
			ESP_LOGI(TAG_TASK_C, "----------Given by C (delay 2100 ms)");
			xSemaphoreGive(semaphore_handle);
			vTaskDelay(pdMS_TO_TICKS(2100));
		}
	}
}

void task_sem_d(void *pvParameters)
{
	static const char *TAG_TASK_D = "Task D";

	while (1)
	{
		if (xSemaphoreTake(semaphore_handle, portMAX_DELAY) == pdTRUE)
		{
			ESP_LOGI(TAG_TASK_D, "---------------Taken by D (delay 100 ms)");
			vTaskDelay(pdMS_TO_TICKS(100));
			ESP_LOGI(TAG_TASK_D, "---------------Given by D (delay 100 ms)");
			xSemaphoreGive(semaphore_handle);
			vTaskDelay(pdMS_TO_TICKS(100));
		}
	}
}

void task_sem_x(void *pvParameters)
{
    static const char *TAG_TASK_X = "Task X";
    uint32_t delay = *(uint32_t *)pvParameters;

    while (1)
    {
        if (xSemaphoreTake(semaphore_handle, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI(TAG_TASK_X, "Taken by %s, core: %d (delay %lu ms)", pcTaskGetName(NULL), xPortGetCoreID(), delay);
            vTaskDelay(pdMS_TO_TICKS(delay));
            ESP_LOGI(TAG_TASK_X, "Given by %s, core: %d (delay %lu ms)", pcTaskGetName(NULL), xPortGetCoreID(), delay);
            xSemaphoreGive(semaphore_handle);
			vTaskDelay(pdMS_TO_TICKS(delay));
        }
		else
        {
            ESP_LOGI(TAG_TASK_X, "%s timeout waiting for semaphore", pcTaskGetName(NULL));
        }
		
    }
}