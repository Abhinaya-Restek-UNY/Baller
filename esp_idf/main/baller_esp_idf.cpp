

#include "UART.hpp"
#include "blink.hpp"
#include "driver/gpio.h"
#include "mpu_to_serial_hub.hpp"

#define MPU_INT_PIN GPIO_NUM_19
UART serialPort(400);

extern "C" void app_main(void) {

  gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);

  if (setup_mpu6050(GPIO_NUM_22, GPIO_NUM_21)) {
    blink_fatal();
  }

  blink_once();
  if (!serialPort.begin()) {
    blink_fatal();
  }

  blink_once();

  if (setup_mpu6050_interrupt(&serialPort, GPIO_NUM_19)) {
    blink_fatal();
  }
  blink_once();

  while (1) {
    vTaskDelay(10);
  }
}
