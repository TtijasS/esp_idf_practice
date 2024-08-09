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

/**
 * @brief Old version of encapsulation handler that failed to manage messages with unclear structure
 *
 * @param uart_num uart port number
 * @param detected_pattern_indices array of pattern indices
 * @param encapsulation_counter number of patterns detected (0, 1 or 2)
 * @param pat_queue_count uart detected patterns counter (for avoiding queue full warning)
 * @return int
 */
int uart_encapsulation_handler(uart_port_t uart_num, int *detected_pattern_indices, int *encapsulation_counter, int *pat_queue_count)
{
	int pat_pos;

	while (((pat_pos = uart_pattern_pop_pos(uart_num)) != -1) && ((*encapsulation_counter) < 2))
	{
		detected_pattern_indices[(*encapsulation_counter)++] = pat_pos;
		(*pat_queue_count)++;
		if (*pat_queue_count == UART_PAT_QUEUE_SIZE)
		{
			uart_pattern_queue_reset(uart_num, UART_PAT_QUEUE_SIZE);
			*pat_queue_count = 0;
		}
	}
	return *encapsulation_counter;
}

/**
 * @brief Encapsulated message start flag handling
 *
 * Confirm the structure of encapsulated messages start flag.
 *
 * @param uart_num uart port number
 * @param encap_start_buf uint8_t encapsulation start flag buffer
 * @param pattern_index starting index of the detected pattern
 * @return 0 OK
 * @return -1 null operator passed
 * @return -2 failed malloc
 * @return -3 not enough bytes in RX buffer
 */
int uart_encapsulation_start_flag_handler(uart_port_t uart_num, uint8_t *encap_start_buf, int pattern_index)
{
	const char *TAG = "ENCAP START";
	if (encap_start_buf == NULL)
	{
		ESP_LOGE(TAG, "Null operators passed to the function");
		return -1;
	}

	int message_size = pattern_index + ENCAPS_FLAG_SIZE;
	uint8_t *tmp_buffer = (uint8_t *)malloc(message_size * sizeof(uint8_t));
	if (tmp_buffer == NULL)
	{
		ESP_LOGE(TAG, "Failed to malloc tmp_buffer");
		return -2;
	}

	if (uart_read_bytes(uart_num, tmp_buffer, message_size, pdMS_TO_TICKS(100)) != message_size)
	{
		ESP_LOGE(TAG, "Wrong message size");
		free(tmp_buffer);
		return -3;
	}

	memcpy(encap_start_buf, &tmp_buffer[pattern_index], ENCAPS_FLAG_SIZE);

	free(tmp_buffer);

	return memcmp(encap_start_buf, ENCAPS_START_PAT, ENCAPS_FLAG_SIZE);
}

/**
 * @brief Encapsulated message end flag handling
 *
 * Confirm the position and structure of the encapsulated messages end flag
 *
 * @param uart_num uart port number
 * @param encap_end_buf uint8_t encapsulation end flag buffer
 * @param pattern_index starting index of the detected pattern
 * @return 0 OK
 * @return -1 null operator passed
 * @return -2 wrong pattern indice
 * @return -3 not enough bytes in RX buffer
 */
int uart_encapsulation_end_flag_handler(uart_port_t uart_num, uint8_t *encap_end_buf, int pattern_index)
{
	const char *TAG = "ENCAP STOP";
	if (encap_end_buf == NULL)
	{
		ESP_LOGE(TAG, "Null operators passed to the function");
		return -1;
	}
	if (pattern_index != 1)
	{
		ESP_LOGE(TAG, "Pat indice != -1");
		return -2;
	}

	if (uart_read_bytes(uart_num, encap_end_buf, ENCAPS_FLAG_SIZE, pdMS_TO_TICKS(100)) != ENCAPS_FLAG_SIZE)
	{
		ESP_LOGE(TAG, "Incorrect message size");
		return -3;
	}

	return memcmp(encap_end_buf, ENCAPS_END_PAT, ENCAPS_FLAG_SIZE);
}

/**
 * @brief Store the encapsulated message
 *
 * @param uart_num uart port number
 * @param message_buf uint8_t message buffer array
 * @param message_size message buffer size
 * @return 0 OK
 * @return -1 null operator passed
 * @return -2 message_size < 0
 * @return -3 not enough bytes in RX buffer
 */
int uart_encapsulated_message_handler(uart_port_t uart_num, uint8_t *message_buf, int message_size)
{
	/*
	This function simply reads the encapsulated message that should sit between two encapsulation flags.
	*/
	const char *TAG = "ENCAP MSG";
	if (message_buf == NULL)
	{
		ESP_LOGE(TAG, "Null operators passed to the function");
		return -1;
	}
	if (message_size < 0)
		return -2;

	if (uart_read_bytes(uart_num, message_buf, message_size, pdMS_TO_TICKS(100)) != message_size)
	{
		ESP_LOGE(TAG, "Wrong message size");
		return -3;
	}
	return 0;
}

int uart_encapsulation_handler(uart_port_t uart_num, int *encap_count, int *pattern_index, uint8_t *encap_start_buf, uint8_t *encap_end_buf, uint8_t *message_buf, int *message_size)
{
	const char* TAG = "ENCAP HANDL"
	// Parse START FLAG
	if (*encap_count == 0)
	{
		if (uart_encapsulation_start_flag_handler(UART_NUM, encap_start_buf, *pattern_index) == 0)
		{
			(*encap_count)++;
		}
		else
		{
			ESP_LOGE(TAG, "Bad start flag");
			*encap_count = 0;
		}
	}
	else if (*encap_count == 1)
	{
		*message_size = *pattern_index - 1;
		if (tmp_message_size < 1)
		{
			ESP_LOGE(TAG, "tmp message size < 1");
			*encap_count = 0;
			break;
		}

		tmp_message_buffer = (uint8_t *)malloc(tmp_message_size * sizeof(uint8_t));
		if (tmp_message_buffer == NULL)
		{
			ESP_LOGE(TAG, "Failed to malloc tmp_message_buffer");
		}

		if (uart_encapsulated_message_handler(UART_NUM, tmp_message_buffer, tmp_message_size) == 0)
		{
			ESP_LOGI(TAG, "Message received OK");
			*encap_count++;
			pat_indice -= tmp_message_size;
			ESP_LOGI(TAG, "New pat indice: %d", pat_indice);
			uart_write_bytes(UART_NUM, tmp_message_buffer, tmp_message_size);
			uart_write_bytes(UART_NUM, "\0", 1);
		}
		else
		{
			ESP_LOGE(TAG, "Failed to receive message");
			*encap_count = 0;
			free(tmp_message_buffer);
		}
		if ((*encap_count > 1) && (uart_encapsulation_end_flag_handler(UART_NUM, encap_stop_flag_buf, pat_indice) == 0))
		{
			ESP_LOGI(TAG, "Message complete");
			uart_write_bytes(UART_NUM, encap_start_flag_buf, ENCAPS_FLAG_SIZE);
			uart_write_bytes(UART_NUM, tmp_message_buffer, tmp_message_size);
			uart_write_bytes(UART_NUM, encap_stop_flag_buf, ENCAPS_FLAG_SIZE);
			*encap_count = 0;
			tmp_message_size = 0;
			uart_pattern_queue_reset(UART_NUM, UART_PAT_QUEUE_SIZE);
		}
		else
		{
			ESP_LOGE(TAG, "Failed stop flag");
			uart_write_bytes(UART_NUM, encap_stop_flag_buf, ENCAPS_FLAG_SIZE);
			uart_write_bytes(UART_NUM, "\0", 1);
			ESP_LOGE(TAG, "(bytes sent)");
			free(tmp_message_buffer);
			*encap_count = 0;
			tmp_message_size = 0;
		}
	}
}

void task_uart_isr_monitoring(void *params)
{
	const char *TAG = "UART ISR TASK";

	uart_event_t uart_event;

	// Buffers for storing start end end flags of encapsulation
	uint8_t *encap_start_flag_buf = (uint8_t *)calloc(ENCAPS_FLAG_SIZE, sizeof(uint8_t));
	uint8_t *encap_stop_flag_buf = (uint8_t *)calloc(ENCAPS_FLAG_SIZE, sizeof(uint8_t));

	int tmp_message_size = 0;
	uint8_t *tmp_message_buffer = NULL;

	int detected_pattern_indices[2] = {0};
	int encapsulation_counter = 0;
	int pattern_queue_counter = 0;

	while (1)
	{
		// Receive the entire uart_event_t structure from the queue
		if (xQueueReceive(uart_event_queue_handle, (void *)&uart_event, portMAX_DELAY) == pdTRUE)
		{
			// Clear the buffer
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

				int pat_indice = uart_pattern_pop_pos(UART_NUM);
				ESP_LOGI(descriptor, "Pat indice: %d", pat_indice);
				if (pat_indice == -1)
				{
					ESP_LOGE(TAG, "Pattern indice -1");
					break;
				}

				// Parse START FLAG
				if (encapsulation_counter == 0)
				{
					if (uart_encapsulation_start_flag_handler(UART_NUM, encap_start_flag_buf, pat_indice) == 0)
					{
						encapsulation_counter++;
						ESP_LOGI(TAG, "Start flag OK");
						uart_write_bytes(UART_NUM, encap_start_flag_buf, ENCAPS_FLAG_SIZE);
						uart_write_bytes(UART_NUM, "\0", 1);
					}
					else
					{
						ESP_LOGE(TAG, "Failed start flag");
					}
				} // PARSE MESSAGE
				else if (encapsulation_counter == 1)
				{
					tmp_message_size = pat_indice - 1;
					if (tmp_message_size < 1)
					{
						ESP_LOGE(TAG, "tmp message size < 1");
						encapsulation_counter = 0;
						break;
					}

					tmp_message_buffer = (uint8_t *)malloc(tmp_message_size * sizeof(uint8_t));
					if (tmp_message_buffer == NULL)
					{
						ESP_LOGE(TAG, "Failed to malloc tmp_message_buffer");
					}

					if (uart_encapsulated_message_handler(UART_NUM, tmp_message_buffer, tmp_message_size) == 0)
					{
						ESP_LOGI(TAG, "Message received OK");
						encapsulation_counter++;
						pat_indice -= tmp_message_size;
						ESP_LOGI(TAG, "New pat indice: %d", pat_indice);
						uart_write_bytes(UART_NUM, tmp_message_buffer, tmp_message_size);
						uart_write_bytes(UART_NUM, "\0", 1);
					}
					else
					{
						ESP_LOGE(TAG, "Failed to receive message");
						encapsulation_counter = 0;
						free(tmp_message_buffer);
					}
					if ((encapsulation_counter > 1) && (uart_encapsulation_end_flag_handler(UART_NUM, encap_stop_flag_buf, pat_indice) == 0))
					{
						ESP_LOGI(TAG, "Message complete");
						uart_write_bytes(UART_NUM, encap_start_flag_buf, ENCAPS_FLAG_SIZE);
						uart_write_bytes(UART_NUM, tmp_message_buffer, tmp_message_size);
						uart_write_bytes(UART_NUM, encap_stop_flag_buf, ENCAPS_FLAG_SIZE);
						encapsulation_counter = 0;
						tmp_message_size = 0;
						uart_pattern_queue_reset(UART_NUM, UART_PAT_QUEUE_SIZE);
					}
					else
					{
						ESP_LOGE(TAG, "Failed stop flag");
						uart_write_bytes(UART_NUM, encap_stop_flag_buf, ENCAPS_FLAG_SIZE);
						uart_write_bytes(UART_NUM, "\0", 1);
						ESP_LOGE(TAG, "(bytes sent)");
						free(tmp_message_buffer);
						encapsulation_counter = 0;
						tmp_message_size = 0;
					}
				}
				break;
			}
			default:
			{
				ESP_LOGI(TAG, "Default switch state");
				break;
			}
			}
		}
	}
	ESP_LOGW(TAG, "KILLING THE UART ISR TASK");
	vTaskDelete(NULL);
}
// 	if (uart_encapsulation_handler(UART_NUM, detected_pattern_indices, &encapsulation_counter, &pattern_queue_counter) == 2)
// 	{
// 		// Get encapsulation indices

// 		int pat_start = detected_pattern_indices[0];
// 		int pat_end = detected_pattern_indices[1] + UART_PATTERN_SIZE - 1;

// 		// Prepare message buffer
// 		size_t message_size = (pat_end - pat_start); /*!< +1 for NULL operator*/
// 		uint8_t *message = calloc(message_size + 1, sizeof(uint8_t));

// 		if (message == NULL)
// 		{
// 			ESP_LOGE(TAG, "Failed to allocate message heap");
// 		}

// 		// Read the RX data up until the end of the second pattern
// 		uart_get_buffered_data_len(UART_NUM, &uart_tmp_buf_size);
// 		uart_read_bytes(UART_NUM, uart_tmp_buffer, pat_end + 1, pdMS_TO_TICKS(100));

// 		// Copy the encapsulated message
// 		memcpy(message, &uart_tmp_buffer[pat_start], message_size * sizeof(uint8_t));
// 		message[message_size + 1] = '\0';
// 		uart_write_bytes(UART_NUM, descriptor, strlen(descriptor));
// 		uart_write_bytes(UART_NUM, (const char *)message, message_size);

// 		if (message[UART_PATTERN_SIZE] == '*' && message[message_size - UART_PATTERN_SIZE] == '*')
// 		{
// 			ESP_LOGI(TAG, "START STOP RECEIVED");
// 			if (memcmp(&message[UART_PATTERN_SIZE], (const void *)"*START*", strlen("*START*")) == 0)
// 			{
// 				ESP_LOGI(TAG, "OK");
// 			}
// 			else
// 			{
// 				ESP_LOGW(TAG, "NOT OK");
// 			}
// 		}
// 		else if (message[message_size - 8])
// 		{
// 			ESP_LOGE(TAG, "WRONG FLAG PLACEMENT");
// 		}
// 		memset(detected_pattern_indices, 0, sizeof(int) * 2);
// 		encapsulation_counter = 0;
// 		free(message); /*!< Free the allocated message buffer */
// 		uart_pattern_queue_reset(UART_NUM, UART_PAT_QUEUE_SIZE);
// 		// uart_write_bytes(UART_NUM, "*OK*", strlen("*OK*"));
// 	}
// 	else if (encapsulation_counter == 1)
// 	{
// 		uart_get_buffered_data_len(UART_NUM, &uart_tmp_buf_size);

// 		if (uart_tmp_buf_size < UART_PATTERN_SIZE + 1)
// 		{
// 			ESP_LOGE(TAG, "Flag is shorter than pattern size + 1");
// 		}

// 		int pat_start = detected_pattern_indices[0];
// 		int pat_end = pat_start + UART_PATTERN_SIZE; /*!< This gives us the */

// 		// Prepare message buffer
// 		size_t message_size = (pat_end - pat_start); /*!< +1 for NULL operator*/
// 		uint8_t *message = calloc(message_size + 1, sizeof(uint8_t));
// 		uart_read_bytes(UART_NUM, message, message_size, pdMS_TO_TICKS(100));

// 		// Prepare message buffer
// 		size_t message_size = (pat_end - pat_start); /*!< +1 for NULL operator*/
// 		uint8_t *message = calloc(message_size + 1, sizeof(uint8_t));

// 		ESP_LOGI(TAG, "Only first flag received");
// 	}
// 	else
// 	{
// 		ESP_LOGI(TAG, "No patterns received");
// 	}
// 	break;
// }
// default:
// {
// 	ESP_LOGI(TAG, "[DEFAULT]: %d", uart_event.type);
// 	break;
// }
// }