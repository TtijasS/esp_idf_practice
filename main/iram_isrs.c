#include "iram_isrs.h"

QueueHandle_t uart_event_queue_handle;

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
	ESP_ERROR_CHECK(uart_driver_install(port_num, rx_buff_size, tx_buff_size, isr_queue_size, &uart_event_queue_handle, intr_alloc_flags));
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

int uart_encapsulation_handler(int *detected_patterns, int *pattern_counter)
{
	int pat_pos;
	while (*pattern_counter < 2)
	{
		pat_pos = uart_pattern_pop_pos(UART_NUM);
		if (pat_pos == -1)
			break;
		detected_patterns[*pattern_counter] = pat_pos;
		++(*pattern_counter);
		ESP_LOGI((const char*)"PDH", "Pat pos %d @ index %d", *pattern_counter, pat_pos);
	}
	return *pattern_counter;
}


void task_uart_isr_monitoring(void *params)
{
	const char *TAG = "UART ISR TASK";

	uart_event_t uart_event;

	size_t uart_tmp_buf_size;
	uint8_t *uart_tmp_buffer = (uint8_t *)calloc(UART_RX_BUFF_SIZE, sizeof(uint8_t));

	int detected_patterns[2] = {0};
	int pattern_counter = 0;

	if (uart_tmp_buffer == NULL)
	{
		ESP_LOGE(TAG, "Failed to malloc rx_data array");
		free(uart_tmp_buffer);
		uart_tmp_buffer = NULL;
		vTaskDelete(NULL);
	}

	while (1)
	{
		// Receive the entire uart_event_t structure from the queue
		if (xQueueReceive(uart_event_queue_handle, (void *)&uart_event, portMAX_DELAY) == pdTRUE)
		{
			// Clear the buffer
			memset(uart_tmp_buffer, 0, sizeof(uint8_t) * UART_RX_BUFF_SIZE);
			ESP_LOGI(TAG, "[EVENT]: %d, [SIZE]: %d", uart_event.type, uart_event.size);

			switch (uart_event.type)
			{
			/**
			 * @brief Pattern detection case
			 *
			 * Multiple identical characters of specific type detected in a row.
			 * Number of chars is defined with macro UART_PATTERN_SIZE
			 *
			 */
			case UART_PATTERN_DET:
			{
				const char *descriptor = "[UART_PAT]: "; /*!<  message description*/
				if (uart_encapsulation_handler(detected_patterns, &pattern_counter) == 2)
				{
					// Get encapsulation indices

					int pat_start = detected_patterns[0];
					int pat_end = detected_patterns[1] + UART_PATTERN_SIZE - 1;

					// Prepare message buffer
					size_t message_size = (pat_end - pat_start); /*!< +1 for NULL operator*/
					uint8_t *message = calloc(message_size + 1, sizeof(uint8_t));

					if (message == NULL)
					{
						ESP_LOGE(TAG, "Failed to allocate message heap");
					}

					// Read the RX data up until the end of the second pattern
					uart_get_buffered_data_len(UART_NUM, &uart_tmp_buf_size);
					uart_read_bytes(UART_NUM, uart_tmp_buffer, pat_end + 1, pdMS_TO_TICKS(100));

					// Copy the encapsulated message
					memcpy(message, &uart_tmp_buffer[pat_start], message_size * sizeof(uint8_t));
					message[message_size + 1] = '\0';
					uart_write_bytes(UART_NUM, descriptor, strlen(descriptor));
					uart_write_bytes(UART_NUM, (const char *)message, message_size);

					if (message[UART_PATTERN_SIZE] == '*' && message[message_size - UART_PATTERN_SIZE] == '*')
					{
						ESP_LOGI(TAG, "START STOP RECEIVED");
						if (memcmp(&message[UART_PATTERN_SIZE], (const void*)"*START*", strlen("*START*")) == 0)
						{
							ESP_LOGI(TAG, "OK");
						}
						else{
							ESP_LOGW(TAG, "NOT OK");
						}
					}
					else if (message[message_size - 8])
					{
						ESP_LOGE(TAG, "WRONG FLAG PLACEMENT");
					}
					memset(detected_patterns, 0, sizeof(int) * 2);
					pattern_counter = 0;
					free(message); /*!< Free the allocated message buffer */
					uart_pattern_queue_reset(UART_NUM, UART_PAT_POS_QUEUE_SIZE);
					// uart_write_bytes(UART_NUM, "*OK*", strlen("*OK*"));
				}
				else if (pattern_counter == 1)
				{
					ESP_LOGI(TAG, "Only first flag received");
				}
				else
				{
					ESP_LOGI(TAG, "No patterns received");
				}
				break;
			}
			default:
			{
				ESP_LOGI(TAG, "[DEFAULT]: %d", uart_event.type);
				break;
			}
			}
		}
	}

	ESP_LOGW(TAG, "KILLING THE UART ISR TASK");
	free(uart_tmp_buffer);
	uart_tmp_buffer = NULL;
	vTaskDelete(NULL);
}
