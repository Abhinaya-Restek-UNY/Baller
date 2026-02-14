#include "driver/i2c_types.h"
#include "soc/gpio_num.h"
#include <sys/_timeval.h>

class I2C {
protected:
  I2C(i2c_master_bus_handle_t bus, uint8_t address);
  ~I2C();

  void writeByte(uint8_t reg, uint8_t byte);
  uint8_t readByte(uint8_t reg);
  int16_t readWord(uint8_t reg);
  void readBytes(uint8_t reg_start, size_t count, uint8_t *dest);

  void wait(uint32_t ms);

private:
  struct timeval tv_now;
  void initI2C(gpio_num_t scl, gpio_num_t sda, uint8_t address);
  void initMPU();

  int64_t getTimeMS();

  i2c_master_dev_handle_t device_handle;

  uint8_t byte_read_buf = 0;
  uint16_t write_reg_tmp = 0;
};
