#include "queue_tasks.h"

QueueHandle_t queue_handle;

void task_main_producer(void *pvParameters)
{
	const char* TAG_PROD = "Producer";

	uint8_t item = 0;
	while (1)
	{
		if (xQueueSend(queue_handle, &item, pdMS_TO_TICKS(1000)) == pdPASS)
		{
			ESP_LOGI(TAG_PROD, "Item %d sent to queue", item);
		}
		else{
			ESP_LOGI(TAG_PROD, "Failed to send item %d", item);
		}
		vTaskDelay(pdMS_TO_TICKS(1000));
		++item;
	}
}

void task_producer_2(void *pvParameters)
{
	const char* TAG_PROD = "Producer";

	uint8_t item = 255;
	while (1)
	{
		if (xQueueSend(queue_handle, &item, pdMS_TO_TICKS(500)) == pdPASS)
		{
			ESP_LOGI(TAG_PROD, "Item %d sent to queue", item);
		}
		else{
			ESP_LOGI(TAG_PROD, "Failed to send item %d", item);
		}
		vTaskDelay(pdMS_TO_TICKS(500));
		--item;
	}
}

void task_consumer(void *pvParameters)
{
	const char* TAG_CONS = "Consumer";
	uint8_t item;
	while (1)
	{
		if (xQueueReceive(queue_handle, &item, portMAX_DELAY) == pdPASS)
		{
			ESP_LOGI(TAG_CONS, "Item %d received from queue", item);
		}
	}
}

void task_consumer_core_0(void *pvParameters)
{
	uint8_t item;

	while (1)
	{
		if (xQueueReceive(queue_handle, &item, portMAX_DELAY) == pdPASS)
		{
			
		}
	}

}