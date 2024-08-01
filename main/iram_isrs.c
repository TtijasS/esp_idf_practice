#include "iram_isrs.h"

QueueHandle_t uart_isr_queue_handle;

uart_config_t uart_config = {
	.baud_rate = UART_BAUD,
	.data_bits = UART_DATA_8_BITS,
	.parity = UART_PARITY_DISABLE,
	.stop_bits = UART_STOP_BITS_1,
	.flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

/**
 * @brief Initi uart with isr queue logging
 *
 * @param uart_config uart config file
 * @param port_num valid uart port number
 * @param gpio_tx tx pin
 * @param gpio_rx rx pin
 * @param rx_buff_size rx buffer size
 * @param tx_buff_size tx buffer size
 * @param isr_queue_handle isr interrupt handle
 * @param isr_queue_size isr queue size (number of interrupt events to store)
 * @param intr_alloc_flags ESP_INTR_FLAG_*
 */
void uart_init_with_isr_queue(uart_config_t *uart_config, uart_port_t port_num, int gpio_tx, int gpio_rx, int tx_buff_size, int rx_buff_size, QueueHandle_t *isr_queue_handle, int isr_queue_size, int intr_alloc_flags)
{
	// Create the ISR queue
	*isr_queue_handle = xQueueCreate(isr_queue_size, sizeof(uart_event_t));
	if (*isr_queue_handle == NULL)
	{
		ESP_LOGE("UART_INIT", "Failed to create ISR queue");
		return;
	}

	ESP_ERROR_CHECK(uart_param_config(port_num, uart_config));
	ESP_ERROR_CHECK(uart_driver_install(port_num, rx_buff_size, tx_buff_size, isr_queue_size, &uart_isr_queue_handle, intr_alloc_flags));
	ESP_ERROR_CHECK(uart_set_pin(port_num, gpio_tx, gpio_rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
}

/**
 * @brief Initi uart with isr queue logging
 *
 * @param uart_config uart config file
 * @param port_num valid uart port number
 * @param gpio_tx tx pin
 * @param gpio_rx rx pin
 * @param tx_buff_size tx buffer size
 * @param rx_buff_size rx buffer size
 */
void uart_init(uart_config_t *uart_config, uart_port_t port_num, int gpio_tx, int gpio_rx, int tx_buff_size, int rx_buff_size)
{
	ESP_ERROR_CHECK(uart_param_config(port_num, uart_config));
	ESP_ERROR_CHECK(uart_driver_install(port_num, rx_buff_size, tx_buff_size, 0, NULL, 0));
	ESP_ERROR_CHECK(uart_set_pin(port_num, gpio_tx, gpio_rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
}

void task_uart_isr_monitoring(void *params)
{
	const char *TAG = "TSK ISR UART";

	uart_event_t uart_event;

	size_t tmp_data_size;
	uint8_t *tmp_data_buffer = (uint8_t *)malloc(sizeof(uint8_t) * UART_RX_BUFF_SIZE);
	if (tmp_data_buffer == NULL)
	{
		ESP_LOGE(TAG, "Failed to malloc rx_data array");
		free(tmp_data_buffer);
		tmp_data_buffer = NULL;
		vTaskDelete(NULL);
	}

	ESP_LOGI(TAG, "UART ISR event monitoring started on core %d", xTaskGetCoreID(NULL));
	while (1)
	{
		if (xQueueReceive(uart_isr_queue_handle, (void *)&uart_event, portMAX_DELAY) == pdTRUE)
		{
			switch (uart_event.type)
			{
			case UART_DATA:
				ESP_LOGI(TAG, "UART data received");
				tmp_data_size = uart_read_bytes(UART_PORT_NUM, tmp_data_buffer, UART_RX_BUFF_SIZE, portMAX_DELAY);
				ESP_LOGI(TAG, "Received data: %u %s", tmp_data_size, tmp_data_buffer);
				break;
			case UART_BUFFER_FULL:
				ESP_LOGI(TAG, "UART buffer full");
				uart_flush_input(UART_PORT_NUM);
				xQueueReset(uart_isr_queue_handle);
				break;
			case UART_FIFO_OVF:
				ESP_LOGI(TAG, "UART FIFO overflow");
				uart_flush_input(UART_PORT_NUM);
				xQueueReset(uart_isr_queue_handle);
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
			case UART_PATTERN_DET:
				uart_get_buffered_data_len(UART_PORT_NUM, &tmp_data_size);
				int pattern_pos = uart_pattern_get_pos(UART_PORT_NUM);

				ESP_LOGI(TAG, "UART Pattern detected, pos: %d, rx_data_size: %u", pattern_pos, tmp_data_size);
				if (pattern_pos == -1)
				{
					uart_flush_input(UART_PORT_NUM);
				}
				else
				{
					uart_read_bytes(UART_PORT_NUM, tmp_data_buffer, pattern_pos, pdMS_TO_TICKS(100));
					uint8_t received_pattern[UART_PATTERN_SIZE + 1]; // number of pattern chars + \0
					memset(received_pattern, 0, sizeof(received_pattern));
					uart_read_bytes(UART_PORT_NUM, received_pattern, UART_PATTERN_SIZE, pdMS_TO_TICKS(100));
					ESP_LOGI(TAG, "data read: %s", tmp_data_buffer);
					ESP_LOGI(TAG, "pattern : %s", received_pattern);
				}
				break;
			default:
				ESP_LOGI(TAG, "UART event type: %d", uart_event.type);
				break;
			}
		}
	}

	free(tmp_data_buffer);
	tmp_data_buffer = NULL;
	vTaskDelete(NULL);
}
