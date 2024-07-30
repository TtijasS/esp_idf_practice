#include "semaphore_tasks.h"

SemaphoreHandle_t semaphore_core_0;
SemaphoreHandle_t semaphore_core_1;

void task_sem_a(void *pvParameters)
{
	static const char *TAG_TASK_A = "Task A";

	while (1)
	{
		if (xSemaphoreTake(semaphore_core_0, portMAX_DELAY) == pdTRUE)
		{
			ESP_LOGI(TAG_TASK_A, "Taken by A (delay 1000 ms)");
			vTaskDelay(pdMS_TO_TICKS(1000));
			ESP_LOGI(TAG_TASK_A, "Given by A (delay 1000 ms)");
			xSemaphoreGive(semaphore_core_0);
			vTaskDelay(pdMS_TO_TICKS(10));
		}
		else
		{
			printf("Task %s waiting", pcTaskGetName(NULL));
		}
	}
}

void task_sem_b(void *pvParameters)
{
	static const char *TAG_TASK_B = "Task B";

	while (1)
	{
		if (xSemaphoreTake(semaphore_core_0, portMAX_DELAY) == pdTRUE)
		{
			ESP_LOGI(TAG_TASK_B, "-----Taken by B (delay 1250 ms)");
			vTaskDelay(pdMS_TO_TICKS(1250));
			ESP_LOGI(TAG_TASK_B, "-----Given by B (delay 1250 ms)");
			xSemaphoreGive(semaphore_core_0);
			vTaskDelay(pdMS_TO_TICKS(10));
		}
		else
		{
			printf("Task %s waiting", pcTaskGetName(NULL));
		}
	}
}

void task_sem_c(void *pvParameters)
{
	static const char *TAG_TASK_C = "Task C";

	while (1)
	{
		if (xSemaphoreTake(semaphore_core_0, portMAX_DELAY) == pdTRUE)
		{
			ESP_LOGI(TAG_TASK_C, "----------Taken by C (delay 2100 ms)");
			vTaskDelay(pdMS_TO_TICKS(2100));
			ESP_LOGI(TAG_TASK_C, "----------Given by C (delay 2100 ms)");
			xSemaphoreGive(semaphore_core_0);
			vTaskDelay(pdMS_TO_TICKS(10));
		}
		else
		{
			printf("Task %s waiting", pcTaskGetName(NULL));
		}
	}
}

void task_sem_d(void *pvParameters)
{
	static const char *TAG_TASK_D = "Task D";

	while (1)
	{
		if (xSemaphoreTake(semaphore_core_0, portMAX_DELAY) == pdTRUE)
		{
			ESP_LOGI(TAG_TASK_D, "---------------Taken by D (delay 100 ms)");
			vTaskDelay(pdMS_TO_TICKS(100));
			ESP_LOGI(TAG_TASK_D, "---------------Given by D (delay 100 ms)");
			xSemaphoreGive(semaphore_core_0);
			vTaskDelay(pdMS_TO_TICKS(10));
		}
		else
		{
			printf("Task %s waiting", pcTaskGetName(NULL));
		}
	}
}

void task_double_core_semaphore(void *pvParameters)
{
    static const char *TAG_TASK_X = "Task X";
    uint32_t delay = *(uint32_t *)pvParameters;
	BaseType_t core_id = xTaskGetCoreID(NULL);

	ESP_LOGI(TAG_TASK_X, "Task %s, Core: %d, Param: %lu", pcTaskGetName(NULL), core_id, delay);


    while (1)
    {
        if ((core_id != 1) && xSemaphoreTake(semaphore_core_0, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI(TAG_TASK_X, "Taken by %s, core: %d @ %d (delay %lu ms)", pcTaskGetName(NULL), core_id, xPortGetCoreID(), delay);
            vTaskDelay(pdMS_TO_TICKS(delay));
            ESP_LOGI(TAG_TASK_X, "Given by %s, core: %d @ %d (delay %lu ms)", pcTaskGetName(NULL), core_id, xPortGetCoreID(), delay);
            xSemaphoreGive(semaphore_core_0);
			vTaskDelay(pdMS_TO_TICKS(delay));
			// taskYIELD();
			if (core_id == 0)
				continue;
        }
        if ((core_id != 0) && xSemaphoreTake(semaphore_core_1, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI(TAG_TASK_X, "Taken by %s, core: %d @ %d (delay %lu ms)", pcTaskGetName(NULL), core_id, xPortGetCoreID(), delay);
            vTaskDelay(pdMS_TO_TICKS(delay));
            ESP_LOGI(TAG_TASK_X, "Given by %s, core: %d @ %d (delay %lu ms)", pcTaskGetName(NULL), core_id, xPortGetCoreID(), delay);
            xSemaphoreGive(semaphore_core_1);
			vTaskDelay(pdMS_TO_TICKS(delay));
			// taskYIELD();
			continue;
        }
		ESP_LOGI(TAG_TASK_X, "%s timeout, cpu %d @ %d", pcTaskGetName(NULL), core_id, xPortGetCoreID());
    }
}

void task_stop_watch(void *pvParameters)
{
	static const char *TAG_TASK_X = "Task X";
	uint32_t delay = *(uint32_t *)pvParameters;
	uint32_t counter = 0;

	while (1)
	{
		if (xSemaphoreTake(semaphore_core_0, portMAX_DELAY) == pdTRUE)
		{
			ESP_LOGI(TAG_TASK_X, "Taken by %s, core: %d (delay %lu ms)", pcTaskGetName(NULL), xPortGetCoreID(), delay);
			if (counter == delay)
			{
				vTaskDelay(pdMS_TO_TICKS(delay));
				ESP_LOGI(TAG_TASK_X, "Given by %s, core: %d (delay %lu ms)", pcTaskGetName(NULL), xPortGetCoreID(), delay);
				xSemaphoreGive(semaphore_core_0);
				vTaskDelay(pdMS_TO_TICKS(delay));
			}
			++counter;
		}
		else
		{
			printf("Task %s waiting", pcTaskGetName(NULL));
		}
	}
}