#include "core.hpp"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_tim.h"
#include "tim.h"
#include "usart.h"

motor_handle_t motor_fl = {
	.htim = &htim9,
	.channel = TIM_CHANNEL_1,
	.GPIOx_in1 = Motor1A_GPIO_Port,
	.pin_in1 = Motor1A_Pin,
	.GPIOx_in2 = Motor1B_GPIO_Port,
	.pin_in2 = Motor1B_Pin,

};
motor_handle_t motor_br = {
	.htim = &htim9,
	.channel = TIM_CHANNEL_2,
	.GPIOx_in1 = Motor2A_GPIO_Port,
	.pin_in1 = Motor2A_Pin,
	.GPIOx_in2 = Motor2B_GPIO_Port,
	.pin_in2 = Motor2B_Pin,

};
motor_handle_t motor_fr = {
	.htim = &htim10,
	.channel = TIM_CHANNEL_1,
	.GPIOx_in1 = Motor3A_GPIO_Port,
	.pin_in1 = Motor3A_Pin,
	.GPIOx_in2 = Motor3B_GPIO_Port,
	.pin_in2 = Motor3B_Pin,

};
motor_handle_t motor_bl = {
	.htim = &htim12,
	.channel = TIM_CHANNEL_2,
	.GPIOx_in1 = Motor4A_GPIO_Port,
	.pin_in1 = Motor4A_Pin,
	.GPIOx_in2 = Motor4B_GPIO_Port,
	.pin_in2 = Motor4B_Pin,

};

static serial_hub_handle_t uart_hub;
static uint32_t timer_overflow_count = 0;
static uint8_t dma_rx_buf[512 * 2];
static uint64_t last_time = 0;
static packet_time time_packet = {.delta = 0, .timestamp = 0, .unit_index = 2};

void setup() {
	time_packet.unit_index = 2;

	HAL_TIM_Base_Start_IT(&htim2);
	serial_hub_initialize(&uart_hub, serial_hub_write_cb, NULL);
	serial_hub_reserve_memory(&uart_hub, sizeof(packet_encoder));

	HAL_UARTEx_ReceiveToIdle_DMA(&huart2, dma_rx_buf, sizeof(dma_rx_buf));

	serial_hub_attach_topic(&uart_hub, MOTOR_PACKET_ID,
							sizeof(packet_motor_direction), NULL,
							serial_hub_motor_direction_cb);

	motor_init(&motor_fr);
	motor_init(&motor_fl);
	motor_init(&motor_br);
	motor_init(&motor_bl);

	HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
	HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
	HAL_TIM_Encoder_Start(&htim8, TIM_CHANNEL_ALL);

	last_time = micros64();
	time_packet.timestamp = last_time;
};

uint16_t encoder_prev_a = 0;
uint16_t encoder_prev_b = 0;
uint16_t encoder_prev_c = 0;
packet_encoder encoder_data = {0, 0, 0, 0};

static inline int32_t update_encoder(uint16_t *previous,
									 TIM_HandleTypeDef *enc) {
	uint16_t current = __HAL_TIM_GET_COUNTER(enc);
	int16_t delta = (int16_t)(current - *previous);
	*previous = current;
	return delta;
};

// uint8_t blink_count = 0;

void loop() {
	// blink_count++;
	// if ((blink_count % 15) == 0) {
	// 	blink_count = 0;
	// 	HAL_GPIO_TogglePin(LED_BUILTIN_GPIO_Port, LED_BUILTIN_Pin);
	// }
	encoder_data.timestamp = micros64();
	time_packet.delta = encoder_data.timestamp - time_packet.timestamp;

	if (time_packet.delta >= 5000000) {
		time_packet.timestamp = encoder_data.timestamp;
		serial_hub_write_topic(&uart_hub, TIME_PACKET_ID,
							   (uint8_t *)&time_packet, sizeof(packet_time));
	}

	encoder_data.deltaA = update_encoder(&encoder_prev_a, &htim3);
	encoder_data.deltaB = update_encoder(&encoder_prev_b, &htim4);
	encoder_data.deltaC = update_encoder(&encoder_prev_c, &htim8);

	serial_hub_write_topic(&uart_hub, ENCODER_PACKET_ID,
						   (uint8_t *)&encoder_data, sizeof(packet_encoder));
	last_time = encoder_data.timestamp;

	HAL_Delay(10);
};

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM2) {
		timer_overflow_count++;
	}
}

uint64_t micros64(void) {
	uint32_t high1, low, high2;

	// Read the values until we are sure an interrupt didn't fire in the middle
	do {
		high1 = timer_overflow_count;
		low = __HAL_TIM_GET_COUNTER(&htim2);
		high2 = timer_overflow_count;
	} while (high1 != high2);

	// Shift the overflow count up by 32 bits, and OR it with the hardware
	// counter
	return ((uint64_t)high1 << 32) | low;
}

void serial_hub_write_cb(void *ctx, uint8_t *data, fsize_t size) {
	HAL_UART_Transmit(&huart2, data, size, 100);
};

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
	if (huart->Instance == USART2) {
		// will not reset because of static keyword.
		static uint16_t old_pos = 0;

		uint16_t curr_pos = Size;

		if (curr_pos != old_pos) {
			if (curr_pos > old_pos) {
				uint16_t chunk_length = curr_pos - old_pos;
				serial_hub_on_read(&uart_hub, &dma_rx_buf[old_pos],
								   chunk_length);
			} else {
				uint16_t length_to_end = sizeof(dma_rx_buf) - old_pos;
				serial_hub_on_read(&uart_hub, &dma_rx_buf[old_pos],
								   length_to_end);

				if (curr_pos > 0) {
					serial_hub_on_read(&uart_hub, &dma_rx_buf[0], curr_pos);
				}
			}

			// Move our read pointer up to match the hardware write pointer
			old_pos = curr_pos;
		}
	}
}

void serial_hub_motor_direction_cb(void *ctx, uint8_t *data, fsize_t size) {
	packet_motor_direction *dir = (packet_motor_direction *)data;
	// int16_t a = 32000;
	if (dir->front_left == 0 || dir->front_right == 0 || dir->back_left == 0 ||
		dir->back_right == 0) {

		HAL_GPIO_TogglePin(LED_BUILTIN_GPIO_Port, LED_BUILTIN_Pin);
	}
	motor_set_direction(&motor_fr, dir->front_right);
	motor_set_direction(&motor_fl, dir->front_left);

	motor_set_direction(&motor_br, dir->back_right);
	motor_set_direction(&motor_bl, dir->back_left);
};
