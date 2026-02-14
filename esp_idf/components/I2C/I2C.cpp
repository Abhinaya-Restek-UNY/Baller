#include "I2C.hpp"
#include "driver/i2c_master.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include <cmath>
#include <sys/time.h>

I2C::I2C(i2c_master_bus_handle_t bus, uint8_t address) {
  i2c_device_config_t dev_conf = {.dev_addr_length = I2C_ADDR_BIT_LEN_7,
                                  .device_address = address,
                                  .scl_speed_hz = 100'000};

  ESP_ERROR_CHECK(
      i2c_master_bus_add_device(bus, &dev_conf, &this->device_handle));
}

void I2C::writeByte(uint8_t reg, uint8_t byte) {
  uint8_t buf[2] = {reg, byte};
  ESP_ERROR_CHECK(i2c_master_transmit(this->device_handle, buf, 2, 1000));
}

uint8_t I2C::readByte(uint8_t reg) {
  ESP_ERROR_CHECK(i2c_master_transmit_receive(this->device_handle, &reg, 1,
                                              &this->byte_read_buf, 1, 1000));
  return this->byte_read_buf;
}

int16_t I2C::readWord(uint8_t reg_start) {
  uint8_t buf[2];
  ESP_ERROR_CHECK(
      i2c_master_transmit_receive(device_handle, &reg_start, 1, buf, 2, 1000));
  return (int16_t)((buf[0] << 8) | buf[1]);
}

void I2C::readBytes(uint8_t reg_start, size_t count, uint8_t *dest) {
  ESP_ERROR_CHECK(i2c_master_transmit_receive(device_handle, &reg_start, 1,
                                              dest, count, 100));
}

int64_t I2C::getTimeMS() {
  gettimeofday(&this->tv_now, NULL);
  gettimeofday(&this->tv_now, NULL);

  return (uint64_t)this->tv_now.tv_sec * 1000ULL +
         this->tv_now.tv_usec / 1000ULL;
}

I2C::~I2C() {
  if (this->device_handle) {
    i2c_master_bus_rm_device(device_handle);
  }
}

void I2C::wait(uint32_t ms) { vTaskDelay(pdMS_TO_TICKS(ms)); }
