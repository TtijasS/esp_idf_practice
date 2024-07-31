#include "notification_tasks.h"

TaskHandle_t notification_by_a;
TaskHandle_t notification_by_b;
TaskHandle_t notify_shared;
// configTASK_NOTIFICATION_ARRAY_ENTRIES

void task_notificator_a(void *pvParameters)
{
	const char *TAG_A = "NTF_A";
	BaseType_t cpu_id = xTaskGetCoreID(NULL);

	ESP_LOGI(TAG_A, "A started on cpu %d", cpu_id);
	while (1)
	{
		ESP_LOGI(TAG_A, "running for 1000 ms");
		vTaskDelay(pdMS_TO_TICKS(1000));
		ESP_LOGI(TAG_A, "finished");
		ESP_LOGI(TAG_A, "sending notification");
		xTaskNotifyGive(notification_by_a);
		ESP_LOGI(TAG_A, "Notification sent. Now waiting for notification");
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

void task_notificator_b(void *pvParameters)
{
	const char *TAG_B = "NTF_____B";
	BaseType_t cpu_id = xTaskGetCoreID(NULL);

	ESP_LOGI(TAG_B, "B started on cpu %d", cpu_id);
	while (1)
	{
		ESP_LOGI(TAG_B, "running for 500 ms");
		vTaskDelay(pdMS_TO_TICKS(500));
		ESP_LOGI(TAG_B, "finished");
		ESP_LOGI(TAG_B, "sending notification");
		xTaskNotifyGive(notification_by_b);
		ESP_LOGI(TAG_B, "Notification sent. Now waiting for notification");
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

void task_how_notify_wait_works(void *pvParameters)
{
	const char *TAG = "NTF X";
	BaseType_t cpu_id = xTaskGetCoreID(NULL);
	uint32_t ntf_val = 0x00;  // this value is overwritten nomatter what so it can be anything

	ESP_LOGI(TAG, "NTF X started on cpu %d", cpu_id);

	uint32_t i = 0x01;
	while (1)
	{
		if (i == 1)
		{
			ESP_LOGI(TAG, "Set to 0x11");
			xTaskNotify(notify_shared, 0x11, eSetBits);
			vTaskDelay(pdMS_TO_TICKS(10));
		}
		if (xTaskNotifyWait(0x01, 0x00, &ntf_val, pdMS_TO_TICKS(1000)) == pdTRUE)
		{
			ESP_LOGI(TAG, "ntf_val == %08X", (unsigned int)ntf_val);
		}
		else{
			ESP_LOGI(TAG, "Reset 0x01");
		}
		if (xTaskNotifyWait(0x01, 0x00, &ntf_val, pdMS_TO_TICKS(1000)) == pdTRUE)
		{
			ESP_LOGI(TAG, "ntf_val == %08X", (unsigned int)ntf_val);
		}
		else{
			ESP_LOGI(TAG, "Reset 0x01");
		}
		if (i == 1)
		{
			ESP_LOGI(TAG, "Notify but don't change");
			xTaskNotify(notify_shared, 0x10, eNoAction);
			vTaskDelay(pdMS_TO_TICKS(10));
		}
		// clear on exit = reset specific task handle notification bits after the result is read
		// we get: 0xF0, 0x01, 0x02, 0x04, 0x08, 0x10,...
		// clear on entry = reset specific task handle notification bits if there is no notification pending
		if (xTaskNotifyWait(0x00, ULONG_MAX, &ntf_val, pdMS_TO_TICKS(1000)) == pdTRUE)
		{
			ESP_LOGI(TAG, "ntf_val == %08X", (unsigned int)ntf_val);
		}
		xTaskNotify(notify_shared, i, eSetBits);
		i <<= 1;

		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void task_bit_ntf_1(void *pvParameters)
{
	const char *TAG = "NTF 1";
	BaseType_t cpu_id = xTaskGetCoreID(NULL);
	uint32_t ntf_val = 0x00;

	ESP_LOGI(TAG, "NTF 1 started on cpu %d", cpu_id);
	// Initial notification to start the process
	ESP_LOGI(TAG, "set 0x01");
	while (1)
	{
		if (xTaskNotifyWait(0x00, ULONG_MAX, &ntf_val, pdMS_TO_TICKS(2000)) == pdTRUE)
		{
			ESP_LOGI(TAG, "Read notification %02X", (unsigned int)ntf_val);
			if (ntf_val == 0x00)
			{
				ESP_LOGI(TAG, "Correct notification %02X", (unsigned int)ntf_val);
				ESP_LOGI(TAG, "Notify 0x01");
				xTaskNotify(notification_by_a, 0x01, eSetBits);
				vTaskDelay(pdMS_TO_TICKS(100));
			}
		}
		else
		{
			ESP_LOGI(TAG, "No notification for 2000 ms");
		}
	}
}

void task_bit_ntf_2(void *pvParameters)
{
	const char *TAG = "-----NTF 2";
	BaseType_t cpu_id = xTaskGetCoreID(NULL);
	ESP_LOGI(TAG, "NTF 2 started on cpu %d", cpu_id);
	uint32_t ntf_val;

	while (1)
	{
		if (xTaskNotifyWait(0x00, ULONG_MAX, &ntf_val, pdMS_TO_TICKS(2500)) == pdTRUE)
		{
			ESP_LOGI(TAG, "Read notification %02X", (unsigned int)ntf_val);
			if (ntf_val == 0x01)
			{
				ESP_LOGI(TAG, "Correct notification %02X", (unsigned int)ntf_val);
				ESP_LOGI(TAG, "Notify 0x00");
				xTaskNotify(notification_by_b, 0x00, eSetBits);
				vTaskDelay(pdMS_TO_TICKS(100));
			}
		}
		else
		{
			ESP_LOGI(TAG, "No notification for 2100 ms");
			if (xTaskNotify(notification_by_b, 0x00, eSetValueWithoutOverwrite) == pdPASS)
			{
				ESP_LOGI(TAG, "Notification eNoAction sent");
			}
			else
			{
				ESP_LOGI(TAG, "Notification not processed yet");
			}
		}
	}
}
