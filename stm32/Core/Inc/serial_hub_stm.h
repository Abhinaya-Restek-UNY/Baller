#ifndef SERIAL_HUB_STM_H
#define SERIAL_HUB_STM_H

#include "motor.h"
#define ENCODER_PACKET_ID 1
#define MOTOR_PACKET_ID 4

typedef struct {
  int32_t revolutionA;
  int32_t revolutionB;
  int32_t revolutionC;
  uint64_t timestamp;
} packet_encoder_data;

typedef struct {
  motor_direction_t front_right;
  motor_direction_t front_left;
  motor_direction_t back_right;
  motor_direction_t back_left;
} packet_motor_direction;

#endif
