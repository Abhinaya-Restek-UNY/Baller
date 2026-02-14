#include "I2C.hpp"
#include <sys/_timeval.h>

struct mpu_data {
  int x;
  int y;
  int z;
};

class MPU6050 : public I2C {
public:
  MPU6050(i2c_master_bus_handle_t bus);
  int8_t init();

  int8_t calibrate();
  int8_t start();
  int8_t stop();

  void debug_print_raw_register();

  void update_sample();

private:
  mpu_data accel_bias{.x = 0, .y = 0, .z = 0};
  mpu_data gyro_bias{.x = 0, .y = 0, .z = 0};
  mpu_data accel_hover{.x = 0, .y = 0, .z = 0};
  mpu_data gyro_hover{.x = 0, .y = 0, .z = 0};

  mpu_data accel_prev{.x = 0, .y = 0, .z = 0};
  mpu_data gyro_prev{.x = 0, .y = 0, .z = 0};

  uint16_t get_fifo_count();

  void calculate_bias(mpu_data &accel_bias, mpu_data &gyro_bias);
  void calculate_deadzone(mpu_data &accel_hover, mpu_data &gyro_hover);

  uint16_t collect_fifo_samples(uint8_t total_sample);
  void read_fifo_sample(mpu_data &accel, mpu_data &gyro);

  void read_sensor_data(mpu_data &accel, mpu_data &gyro);

  void reset_device();
};
