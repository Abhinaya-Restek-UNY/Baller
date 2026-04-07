#pragma once

#include <stdint.h>
#define ENCODER_PACKET_ID 1
#define MPU_PACKET_ID 2
#define TIME_PACKET_ID 3
#define MOTOR_PACKET_ID 4

typedef int16_t motor_direction_t;

typedef struct __attribute__((packed)) {
  int16_t front_left;
  int16_t front_right;
  int16_t back_left;
  int16_t back_right;
} packet_motor;

typedef struct __attribute__((packed)) {
  uint64_t timestamp;
  int32_t deltaA;
  int32_t deltaB;
  int32_t deltaC;
} packet_encoder;

typedef struct __attribute__((packed)) {
  float q_w;
  float q_x;
  float q_y;
  float q_z;
  int a_x;
  int a_y;
  int a_z;
  uint64_t timestamp;
} packet_mpu;

typedef struct __attribute__((packed)) {
  uint64_t delta;
  uint64_t timestamp;
  uint8_t unit_index;
} packet_time;
