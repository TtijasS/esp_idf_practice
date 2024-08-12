#include "uart_isr_handler.h"

QueueHandle_t uart_event_queue_handle;
int tmp_message_size = 0;
uint8_t *tmp_message_buffer = NULL;

QueueHandle_t queue_msg_handle;

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
 * @param encap_stop_buf uint8_t encapsulation end flag buffer
 * @param pattern_index starting index of the detected pattern
 * @return 0 OK
 * @return -1 null operator passed
 * @return -2 wrong pattern indice
 * @return -3 not enough bytes in RX buffer
 */
int uart_encapsulation_end_flag_handler(uart_port_t uart_num, uint8_t *encap_stop_buf, int pattern_index)
{
	const char *TAG = "ENCAP STOP";
	if (encap_stop_buf == NULL)
	{
		ESP_LOGE(TAG, "Null operators passed to the function");
		return -1;
	}
	if (pattern_index != 1)
	{
		ESP_LOGE(TAG, "Pat indice != -1");
		return -2;
	}

	if (uart_read_bytes(uart_num, encap_stop_buf, ENCAPS_FLAG_SIZE, pdMS_TO_TICKS(100)) != ENCAPS_FLAG_SIZE)
	{
		ESP_LOGE(TAG, "Incorrect message size");
		return -3;
	}

	return memcmp(encap_stop_buf, ENCAPS_END_PAT, ENCAPS_FLAG_SIZE);
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

/**
 * @brief Handles the state of the received message and manages the UART communication flow
 *
 * Function checks if the successfull start and stop flags have been received and if the received message was complete.
 *
 * @param uart_num uart port number
 * @param encap_state encapsulation state counter (nothing happened = 0, start flag received = 1, stop flag received = 2)
 * @param pattern_index index of the detected pattern
 * @param encap_start_buf debug buffer for checking how the received start flag looks like
 * @param encap_stop_buf debuf buffer for checking how the stop flag looks like
 * @param message_buf received message buffer
 * @param message_size received message size
 * @return 1 start flag ok message received successfully
 * @return 0 message received ok
 * @return -1 bad start flag
 * @return -2 message size < 1
 * @return -3 failed to malloc message buffer
 * @return -4 failed to to read the message from RX buffer
 * @return -5 bad stop flag
 */
int uart_encapsulation_handler(uart_port_t uart_num, int *encap_state, int *pattern_index, uint8_t *encap_start_buf, uint8_t *encap_stop_buf)
{
	const char *TAG = "ENCAP HANDLER";
	uint8_t *tmp_message_buf = NULL;
	int tmp_message_size = 0;

	// Parse START FLAG
	if (*encap_state == 0)
	{
		if (uart_encapsulation_start_flag_handler(UART_NUM, encap_start_buf, *pattern_index) == 0)
		{
			// ESP_LOGI(TAG, "Start flag received");
			(*encap_state)++;
			return 1;
		}
		else
		{
			// ESP_LOGE(TAG, "Bad start flag");
			*encap_state = 0;
			return -1;
		}
	}
	else if (*encap_state == 1)
	{
		tmp_message_size = *pattern_index - 1;

		if (tmp_message_size < 1)
		{
			// ESP_LOGE(TAG, "Message size < 1");
			*encap_state = 0;
			return -2;
		}

		tmp_message_buf = (uint8_t *)malloc(tmp_message_size * sizeof(uint8_t));
		if (tmp_message_buf == NULL)
		{
			// ESP_LOGE(TAG, "Failed to malloc tmp data buffer");
			*encap_state = 0;
			return -3;
		}

		if (uart_encapsulated_message_handler(UART_NUM, tmp_message_buf, tmp_message_size) == 0)
		{
			// ESP_LOGI(TAG, "Message received");
			(*encap_state)++;
			*pattern_index -= tmp_message_size;
		}
		else
		{
			// ESP_LOGE(TAG, "Failed to receive message");
			*encap_state = 0;
			free(tmp_message_buf);
			return -4;
		}
		if ((*encap_state == 2) && (uart_encapsulation_end_flag_handler(UART_NUM, encap_stop_buf, *pattern_index) == 0))
		{
			// TaskQueueMessage_type queue_message = {
			// 	.msg_size = tmp_message_size,
			// 	.msg_ptr = tmp_message_buf
			// };
			// if (xQueueSend(queue_msg_handle, &queue_message, portMAX_DELAY) == pdPASS)
			// {
			// 	free(tmp_message_buf);
			// 	tmp_message_buf = NULL;
			// 	return -6;
			// }
			ESP_LOGI(TAG, "Message complete");
			uart_write_bytes(UART_NUM, encap_start_buf, ENCAPS_FLAG_SIZE);
			uart_write_bytes(UART_NUM, tmp_message_buf, (size_t)tmp_message_size);
			uart_write_bytes(UART_NUM, encap_stop_buf, ENCAPS_FLAG_SIZE);

			// Clear everything
			free(tmp_message_buf);
			tmp_message_buf = NULL;
			*encap_state = 0;
			uart_pattern_queue_reset(UART_NUM, UART_PAT_QUEUE_SIZE);
		}
		else
		{
			// ESP_LOGE(TAG, "Failed stop flag");
			free(tmp_message_buf);
			*encap_state = 0;
			uart_pattern_queue_reset(UART_NUM, UART_PAT_QUEUE_SIZE);
			return -5;
		}
	}
	return 0;
}

void task_uart_isr_monitoring(void *params)
{
	const char *TAG = "UART ISR TASK";

	uart_event_t uart_event;
	int error_flag = 0;
	int encapsulation_counter = 0;

	// Buffers for storing start end end flags of encapsulation
	uint8_t *encap_start_flag_buf = (uint8_t *)calloc(ENCAPS_FLAG_SIZE, sizeof(uint8_t));
	uint8_t *encap_stop_flag_buf = (uint8_t *)calloc(ENCAPS_FLAG_SIZE, sizeof(uint8_t));

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

				int pattern_index = uart_pattern_pop_pos(UART_NUM);
				ESP_LOGI(descriptor, "Pat index: %d", pattern_index);
				if (pattern_index == -1)
				{
					ESP_LOGE(TAG, "Pattern index -1");
					break;
				}

				error_flag = uart_encapsulation_handler(UART_NUM, &encapsulation_counter, &pattern_index, encap_start_flag_buf, encap_stop_flag_buf);

				if (error_flag == 0)
				{
					ESP_LOGI(TAG, "Successfull message: %d", error_flag);
					// TaskQueueMessage_type queue_message;
					// queue_message.msg_ptr = tmp_message_buffer;
					// queue_message.msg_size = (size_t)tmp_message_size;
					// // Send constructed message to the queue
					// xQueueSend(queue_msg_handle, &queue_message, portMAX_DELAY);
				}
				else
				{
					ESP_LOGE(TAG, "Error flag: %d", error_flag);
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

void task_receive_message(void *params)
{
	const char *TAG = "RECEIVE MSG";
	TaskQueueMessage_type enqueued_message;

	while (1)
	{
		if (xQueueReceive(queue_msg_handle, &enqueued_message, pdMS_TO_TICKS(100)))
		{
			if (enqueued_message.msg_ptr == NULL)
			{
				ESP_LOGE(TAG, "Null pointer passed");
				free(enqueued_message.msg_ptr);
			}
			ESP_LOGI(TAG, "Sending message");
			// Write the message
			uart_write_bytes(UART_NUM, enqueued_message.msg_ptr, enqueued_message.msg_size);

			// Free the message memory
			free(enqueued_message.msg_ptr);

			// Reset the data
			enqueued_message.msg_ptr = NULL;
			enqueued_message.msg_size = 0;
		}
	}
}