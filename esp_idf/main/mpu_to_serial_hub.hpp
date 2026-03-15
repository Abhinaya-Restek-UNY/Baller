#pragma once
#include "MPU6050.h"
#include "UART.hpp"
#include "driver/gpio.h"
#include "freertos/idf_additions.h"
#include "helper_3dmath.h"
#include "serial_hub.h"
#include <stdint.h>

#define MPU_PACKET_ID 2

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
// static uint32_t prev;
static uint8_t *fifo_buffer;

static Quaternion quat;

static VectorInt16 accel_raw;

static VectorFloat gravity;
static VectorInt16 accel_real;

static TaskHandle_t mpuTaskHandle = NULL;

int8_t setup_mpu6050(gpio_num_t SCL, gpio_num_t SDA);

int8_t setup_mpu6050_interrupt(serial_hub_handle_t *io,
                               gpio_num_t interrupt_pin);
