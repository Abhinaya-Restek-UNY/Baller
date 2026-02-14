#include "MPU6050.hpp"
#include "esp_err.h"
#include "esp_log.h"
#include "mpu6050_types.hpp"
#include <cmath>
#include <sys/time.h>
#define ANGLE_TRESHOLD 0.8

#define MPU6050_WHOAMI_DEFAULT_VALUE 0x68
#define MPU6050_TAG "MPU6050"

/*
 * Scraped from FastIMU/src/F_MPU6050.cpp with some additional shit.
 * https://github.com/LiquidCGS/FastIMU/
 */

inline int16_t twos_complement(uint8_t &high_byte, uint8_t &low_byte) {
  return (((int16_t)(high_byte)) << 8) | low_byte;
};

MPU6050::MPU6050(i2c_master_bus_handle_t bus)
    : I2C(bus, MPU6050_Regs::DEFAULT_ADDRESS) {

      };

int8_t MPU6050::init() {
  uint8_t address = this->readByte(MPU6050_Regs::WHO_AM_I);
  if (address != MPU6050_WHOAMI_DEFAULT_VALUE) {
    ESP_LOGE(MPU6050_TAG, "Error, invalid address 0x%x", address);
    return -1;
  }

  return 0;
}
void MPU6050::debug_print_raw_register() {
  uint8_t data[2];

  // 0x3B is ACCEL_XOUT_H (The starting register for Accel X)
  readBytes(0x3B + 2, 2, data);

  int16_t raw_value = (data[0] << 8) | data[1];

  // Read the Power Management Register to see if it's sleeping
  uint8_t pwr_mgmt = readByte(MPU6050_Regs::PWR_MGMT_1);

  ESP_LOGW("DIAGNOSTIC", "Raw Accel X: %d | Power Reg: 0x%X", raw_value,
           pwr_mgmt);
}

int8_t MPU6050::start() {
  ESP_LOGI(MPU6050_TAG, "Starting MPU..");
  this->reset_device();
  this->writeByte(MPU6050_Regs::PWR_MGMT_1, 0x0); // clear sleep mode

  // get stable clock source
  this->writeByte(MPU6050_Regs::PWR_MGMT_1,
                  MPU6050_Regs::PWR_MGMT_1_FLAG::CLKSEL_PLL_Z);

  // set dlpf config 44hz on accelerometer and 42hz on the gyro
  // TODO: Might need some tuning
  this->writeByte(MPU6050_Regs::MPU_CONFIG,
                  MPU6050_Regs::MPU_CONFIG_FLAG::DLPF_CFG3);

  // Set sample rate = gyroscope output rate/(1 + SMPLRT_DIV). set to 3 we will
  // get 250hz may need some tuning.
  this->writeByte(MPU6050_Regs::SMPLRT_DIV, 3);

  // this clears out self tests. might need some tuning
  this->writeByte(MPU6050_Regs::ACCEL_CONFIG,
                  MPU6050_Regs::ACCEL_CONFIG_FLAG::AFS_SEL_8G);
  this->writeByte(MPU6050_Regs::GYRO_CONFIG,
                  MPU6050_Regs::GYRO_CONFIG_FLAG::FS_SEL_1000);

  // NOTE: I HAVE NO IDEA WHY FASTIMU DOES THIS TWICE BUT JUST TO BE SURE IMMA
  // DO IT TWICE TOO.
  this->writeByte(MPU6050_Regs::MPU_CONFIG,
                  MPU6050_Regs::MPU_CONFIG_FLAG::DLPF_CFG3);
  // Configure Interrupts and Bypass Enable
  // Set interrupt pin active high, push-pull, hold interrupt pin level HIGH
  // until interrupt cleared, clear on read of INT_STATUS, and enable
  // I2C_BYPASS_EN so additional chips can join the I2C bus

  this->writeByte(MPU6050_Regs::INT_PIN_CFG,
                  MPU6050_Regs::INT_PIN_CFG_FLAG::I2C_BYPASS_EN |
                      MPU6050_Regs::INT_PIN_CFG_FLAG::LATCH_INT_EN);

  this->writeByte(MPU6050_Regs::INTERRUPT_ENABLE,
                  MPU6050_Regs::INT_ENABLE_FLAG::DATA_RDY_EN);
  ESP_LOGI(MPU6050_TAG, "MPU Started");

  this->accel_prev = {0, 0, 0};
  this->gyro_prev = {0, 0, 0};
  return 0;
}

int8_t MPU6050::calibrate() {
  ESP_LOGI(MPU6050_TAG, "Calibrating...");

  this->reset_device();
  ESP_LOGI(MPU6050_TAG, "Device is reset.");
  // get a stable clock source
  this->writeByte(MPU6050_Regs::PWR_MGMT_1,
                  MPU6050_Regs::PWR_MGMT_1_FLAG::CLKSEL_PLL_Z);

  // make sure it finish.
  this->wait(200);

  this->writeByte(MPU6050_Regs::INTERRUPT_ENABLE, false);
  this->writeByte(MPU6050_Regs::FIFO_ENANBLE, false);

  // disable i2c master of the sensor.
  this->writeByte(MPU6050_Regs::I2C_MST_CTRL, false);

  this->writeByte(
      MPU6050_Regs::MPU_CONFIG,
      MPU6050_Regs::MPU_CONFIG_FLAG::EXT_SYNC_SET_INPUT_DISABLED |
          MPU6050_Regs::MPU_CONFIG_FLAG::DLPF_CFG1); //  low pas filter to 188hz

  this->writeByte(
      MPU6050_Regs::GYRO_CONFIG,
      MPU6050_Regs::GYRO_CONFIG_FLAG::FS_SEL_250); // set sensitivity max
  this->writeByte(
      MPU6050_Regs::ACCEL_CONFIG,
      MPU6050_Regs::ACCEL_CONFIG_FLAG::AFS_SEL_2G); // set sensitivity max

  ESP_LOGI(MPU6050_TAG, "Calculating bias");
  this->calculate_bias(accel_bias, gyro_bias);

  ESP_LOGI(MPU6050_TAG, "Calculating hover");
  this->calculate_deadzone(accel_hover, gyro_hover);

  ESP_LOGI(MPU6050_TAG, "accel_bias: x: %d, y: %d, z: %d", accel_bias.x,
           accel_bias.y, accel_bias.z);
  ESP_LOGI(MPU6050_TAG, "gyro_bias: x: %d, y: %d, z: %d", gyro_bias.x,
           gyro_bias.y, gyro_bias.z);

  ESP_LOGI(MPU6050_TAG, "accel_hover: x: %d, y: %d, z: %d", accel_hover.x,
           accel_hover.y, accel_hover.z);
  ESP_LOGI(MPU6050_TAG, "gyro_hover: x: %d, y: %d, z: %d", gyro_hover.x,
           gyro_hover.y, gyro_hover.z);
  // reset to normal state;
  this->reset_device();

  return 0;
}

/*
 * Reset device and wait till it turns on(non sexually).
 */
void MPU6050::reset_device() {
  this->writeByte(MPU6050_Regs::PWR_MGMT_1,
                  MPU6050_Regs::PWR_MGMT_1_FLAG::DEVICE_RESET);

  // make sure the fucking thing is on
  this->wait(100);
}

uint16_t MPU6050::collect_fifo_samples(uint8_t total_sample) {

  // Disable FIFO
  this->writeByte(MPU6050_Regs::USER_CTRL, 0x0);
  // Reset
  writeByte(MPU6050_Regs::USER_CTRL, MPU6050_Regs::USER_CTRL_FLAG::FIFO_RESET);

  // enable fifo for Accelerometer and Gyro
  this->writeByte(MPU6050_Regs::USER_CTRL,
                  MPU6050_Regs::USER_CTRL_FLAG::FIFO_EN);

  this->writeByte(MPU6050_Regs::FIFO_ENANBLE,
                  MPU6050_Regs::FIFO_EN_FLAGS::ACCEL_FIFO_EN |
                      MPU6050_Regs::FIFO_EN_FLAGS::XG_FIFO_EN |
                      MPU6050_Regs::FIFO_EN_FLAGS::YG_FIFO_EN |
                      MPU6050_Regs::FIFO_EN_FLAGS::ZG_FIFO_EN);
  this->wait(total_sample); // wait for 40 sample 40ms/1khz = 1mm

  // stop fifo
  this->writeByte(MPU6050_Regs::FIFO_ENANBLE, false);

  return get_fifo_count();
}

uint16_t MPU6050::get_fifo_count() {

  uint16_t packet_count;
  // get packet count
  this->readBytes(MPU6050_Regs::FIFO_COUNTH, 2, (uint8_t *)&packet_count);

  return twos_complement(((uint8_t *)&packet_count)[0],
                         ((uint8_t *)&packet_count)[1]) /
         12;
};

// WARN: This assume that we have a sample.
void MPU6050::read_fifo_sample(mpu_data &accel, mpu_data &gyro) {

  uint8_t data[12] = {0};

  readBytes(MPU6050_Regs::FIFO_R_W, 12, &data[0]); // read data for averaging
  accel.x = twos_complement(data[0],
                            data[1]); // Form signed 16-bit integer for
                                      // each sample in FIFO
  accel.y = twos_complement(data[2], data[3]);
  accel.z = twos_complement(data[4], data[5]);
  gyro.x = twos_complement(data[6], data[7]);
  gyro.y = twos_complement(data[8], data[9]);
  gyro.z = twos_complement(data[10], data[11]);
}

/*
 *
 */
void MPU6050::calculate_bias(mpu_data &accel_bias, mpu_data &gyro_bias) {

  accel_bias.x = 0;
  accel_bias.y = 0;
  accel_bias.z = 0;

  gyro_bias.x = 0;
  gyro_bias.y = 0;
  gyro_bias.z = 0;

  uint16_t packet_count = this->collect_fifo_samples(40);
  // calculate how many packet.

  int64_t accel_bias_x = 0;
  int64_t accel_bias_y = 0;
  int64_t accel_bias_z = 0;

  int64_t gyro_bias_x = 0;
  int64_t gyro_bias_y = 0;
  int64_t gyro_bias_z = 0;

  mpu_data accel;
  mpu_data gyro;
  for (uint16_t i = 0; i < packet_count; i++) {
    read_fifo_sample(accel, gyro);
    accel_bias_x += accel.x;
    accel_bias_y += accel.y;
    accel_bias_z += accel.z - 16384;

    gyro_bias_x += gyro.x;
    gyro_bias_y += gyro.y;
    gyro_bias_z += gyro.z;
  }

  // DIVIDE BY 4 cus we convert 2g to 8g
  accel_bias.x = (accel_bias_x / packet_count) >> 2;
  accel_bias.y = (accel_bias_y / packet_count) >> 2;
  accel_bias.z = (accel_bias_z / packet_count) >> 2;

  gyro_bias.x = (gyro_bias_x / packet_count) >> 2;
  gyro_bias.y = (gyro_bias_y / packet_count) >> 2;
  gyro_bias.z = (gyro_bias_z / packet_count) >> 2;
}

inline void min_max_compare(mpu_data &min, mpu_data &max, mpu_data &current) {
  if (current.x < min.x) {
    min.x = current.x;
  } else if (current.x > max.x) {
    max.x = current.x;
  }

  if (current.y < min.y) {
    min.y = current.y;
  } else if (current.y > max.y) {
    max.y = current.y;
  }

  if (current.z < min.z) {
    min.z = current.z;
  } else if (current.z > max.z) {
    max.z = current.z;
  }
}

void MPU6050::calculate_deadzone(mpu_data &accel_deadzone,
                                 mpu_data &gyro_deadzone) {
  mpu_data min_accel, max_accel, min_gyro, max_gyro, current_accel,
      current_gyro;

  uint16_t total_sample = collect_fifo_samples(40);

  read_fifo_sample(min_accel, min_gyro);
  max_accel = min_accel;
  max_gyro = min_gyro;

  for (uint16_t i = 1; i < total_sample; i++) {
    read_fifo_sample(current_accel, current_gyro);
    min_max_compare(min_accel, max_accel, current_accel);
    min_max_compare(min_gyro, max_gyro, current_gyro);
  }

  // these values are always positive so we can divide by 2 and divide by 4(to
  // convert 250deg/s to 1000deg/s) by bit shift by 3
  accel_deadzone.x = 2 + (((max_accel.x - min_accel.x)) >> 3);
  accel_deadzone.y = 2 + (((max_accel.y - min_accel.y)) >> 3);
  accel_deadzone.z = 2 + (((max_accel.z - min_accel.z)) >> 3);

  gyro_deadzone.x = 2 + (((max_gyro.x - min_gyro.x)) >> 3);
  gyro_deadzone.y = 2 + (((max_gyro.y - min_gyro.y)) >> 3);
  gyro_deadzone.z = 2 + (((max_gyro.z - min_gyro.z)) >> 3);
}

// THESE ARE 87.5/12.5 LPF. might need some tunning but this is the fastest way
// of doing it.
inline int accel_lpf(int prev, int cur) { return ((cur * 7) + prev) >> 3; };

void MPU6050::update_sample() {
  // WARN: WE MUST CHECK THIS.
  uint8_t status = readByte(MPU6050_Regs::INT_STATUS);

  // 4. Now read your sensor data
  // (Only read if the Data Ready bit (bit 0) is actually set)
  if (!(status & 0x01)) {
    return;
  }

  mpu_data accel = {.x = 0, .y = 0, .z = 0};
  mpu_data gyro = {.x = 0, .y = 0, .z = 0};
  this->read_sensor_data(accel, gyro);

  accel.x -= this->accel_bias.x;
  accel.y -= this->accel_bias.y;
  accel.z -= this->accel_bias.z;

  accel.x = accel_lpf(this->accel_prev.x, accel.x);
  accel.y = accel_lpf(this->accel_prev.y, accel.y);
  accel.z = accel_lpf(this->accel_prev.z, accel.z);

  this->accel_prev = accel;

  gyro.x -= this->gyro_bias.x;
  gyro.y -= this->gyro_bias.y;
  gyro.z -= this->gyro_bias.z;

  if (std::abs(accel.x) < accel_hover.x)
    accel.x = 0;
  if (std::abs(accel.y) < accel_hover.y)
    accel.y = 0;

  // 2. APPLY GYRO DEADZONE (Crucial for preventing drift)
  if (std::abs(gyro.x) < gyro_hover.x)
    gyro.x = 0;
  if (std::abs(gyro.y) < gyro_hover.y)
    gyro.y = 0;
  if (std::abs(gyro.z) < gyro_hover.z)
    gyro.z = 0;

  // Don't deadzone Z! It always has gravity (~4096), so it will never be <
  // deadzone.

  ESP_LOGI(MPU6050_TAG, "gyro: x: %d, y: %d, z: %d", gyro.x, gyro.y, gyro.z);
};

void MPU6050::read_sensor_data(mpu_data &accel, mpu_data &gyro) {

  uint8_t raw_data[14];
  readBytes(MPU6050_Regs::ACCEL_XOUT_H, 14, raw_data);

  accel.x = twos_complement(raw_data[0], raw_data[1]);
  accel.y = twos_complement(raw_data[2], raw_data[3]);
  accel.z = twos_complement(raw_data[4], raw_data[5]);
  // skip we dont need temprature
  gyro.x = twos_complement(raw_data[8], raw_data[9]);
  gyro.y = twos_complement(raw_data[10], raw_data[11]);
  gyro.z = twos_complement(raw_data[12], raw_data[13]);
}
