#include "stm32_forward.hpp"
#include "blink.hpp"

void stm32_forward_setup(UART *usb, UART *stm32) {

  if (!stm32->begin(115200, 17, 16)) {
    blink_fatal();
  }

  stm32->setReceiveCallback(
      [usb](uint8_t *data, uint16_t size) { usb->write(data, size); });

  usb->setReceiveCallback(
      [stm32](uint8_t *data, uint16_t size) { stm32->write(data, size); });
}
