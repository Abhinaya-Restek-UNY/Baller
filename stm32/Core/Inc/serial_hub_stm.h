#ifndef SERIAL_HUB_STM_H
#define SERIAL_HUB_STM_H

#include "motor.h"
#define ENCODER_PACKET_ID 1
#define MOTOR_PACKET_ID 4

typedef struct {
	uint64_t timestamp;
	int32_t revolutionA;
	int32_t revolutionB;
	int32_t revolutionC;
} packet_encoder_data;

typedef struct __attribute__((packed)) {
	int16_t front_left;
	int16_t front_right;
	int16_t back_left;
	int16_t back_right;
} packet_motor;

#endif
