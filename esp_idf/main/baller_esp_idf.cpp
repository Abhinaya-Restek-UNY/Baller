#include "UART.hpp"
#include "blink.hpp"
#include "driver/gpio.h"
#include "mpu_to_serial_hub.hpp"
#include "serial_hub.h"
#include "stm32_forward.hpp"
#include "timetell.hpp"

#define MPU_INT_PIN GPIO_NUM_19
UART usb_io(400);
UART stm32_io(400, UART_NUM_2);
static serial_hub_handle_t serial_hub;

void write_cb(UART *ser_port, uint8_t *data, fsize_t size) {
  ser_port->write(data, size);
}
inline void print_hex_dump(const uint8_t *data, size_t size) {
  for (fsize_t i = 0; i < size; i++) {
    printf("%02X ", data[i]);

    if ((i + 1) % 16 == 0) {
      printf("\n");
    }
  }
  if (size % 16 != 0) {
    printf("\n");
  }
}

extern "C" void app_main(void) {

  gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);

  if (setup_mpu6050(GPIO_NUM_22, GPIO_NUM_21)) {
    blink_fatal();
  }

  blink_once();
  if (!usb_io.begin()) {
    blink_fatal();
  }

  blink_once();
  //
  serial_hub_initialize(&serial_hub, (write_cb_t)write_cb, &usb_io);
  serial_hub_reserve_memory(&serial_hub, sizeof(packet_mpu));
  //
  if (setup_mpu6050_interrupt(&serial_hub, GPIO_NUM_19)) {
    blink_fatal();
  }
  //
  // blink_once();
  //
  start_telling_time(&serial_hub);
  stm32_forward_setup(&usb_io, &stm32_io);
  while (1) {
    vTaskDelay(10);
  }
}
