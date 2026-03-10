#pragma once
#include "MPU6050.h"
#include "UART.hpp"
#include "driver/gpio.h"
#include "freertos/idf_additions.h"
#include "helper_3dmath.h"
#include "serial_hub.h"
#include <stdint.h>

// static uint32_t prev;
static uint8_t *fifo_buffer;

static Quaternion quat;

static VectorInt16 accel_raw;

static VectorFloat gravity;
static VectorInt16 accel_real;

static serial_hub_handle_t serial_hub;

static TaskHandle_t mpuTaskHandle = NULL;

int8_t setup_mpu6050(gpio_num_t SCL, gpio_num_t SDA);

int8_t setup_mpu6050_interrupt(UART *io, gpio_num_t interrupt_pin);
