#include "iram_isrs.h"

QueueHandle_t uart_event_queue;

const uart_config_t uart_config = {
	.baud_rate = UART_BAUD,
	.data_bits = UART_DATA_8_BITS,
	.parity = UART_PARITY_DISABLE,
	.stop_bits = UART_STOP_BITS_1,
	.flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

void uart_init(uart_config_t *uart_config)
{
	ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, uart_config));
	ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, UART_RX_BUFF_SIZE * 4, UART_TX_BUFF_SIZE, UART_EVENT_QUEUE_SIZE, &uart_event_queue, 0));
	ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TX_GPIO, UART_RX_GPIO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
}

void task_uart_isr_monitoring(void *params)
{
	const char *TAG = "TSK ISR UART";

	uart_event_t uart_event;

	uint32_t rx_len = 0;
	uint8_t *rx_data;
	rx_data = (uint8_t *)malloc(sizeof(uint8_t)*UART_RX_BUFF_SIZE);
	if (rx_data == NULL)
	{
		ESP_LOGE(TAG, "Failed to malloc rx_data array");
		free(rx_data);
		rx_data = NULL;
		vTaskDelete(NULL);
	}

	ESP_LOGI(TAG, "UART ISR event monitoring started on core %d", xTaskGetCoreID(NULL));
	while (1)
	{
		if (xQueueReceive(uart_event_queue, (void *)&uart_event, portMAX_DELAY))
		{
			switch (uart_event.type)
			{
			case UART_DATA:
				ESP_LOGI(TAG, "UART data received");
				rx_len = uart_read_bytes(UART_PORT_NUM, rx_data, UART_RX_BUFF_SIZE, portMAX_DELAY);
				ESP_LOGI(TAG, "Received data: %lu %s", rx_len, rx_data);
				break;
			case UART_BUFFER_FULL:
				ESP_LOGI(TAG, "UART buffer full");
				uart_flush_input(UART_PORT_NUM);
				xQueueReset(uart_event_queue);
				break;
			case UART_FIFO_OVF:
				ESP_LOGI(TAG, "UART FIFO overflow");
				uart_flush_input(UART_PORT_NUM);
				xQueueReset(uart_event_queue);
				break;
			case UART_BREAK:
				ESP_LOGI(TAG, "UART break detected");
				break;
			case UART_PARITY_ERR:
				ESP_LOGE(TAG, "UART parity error");
				break;
			case UART_FRAME_ERR:
				ESP_LOGE(TAG, "UART frame error");
				break;
			default:
				ESP_LOGI(TAG, "UART event type: %d", uart_event.type);
				break;
			}
		}
	}

	free(rx_data);
	rx_data = NULL;
	vTaskDelete(NULL);
}
