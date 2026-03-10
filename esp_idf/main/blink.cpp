#include "blink.hpp"
#include "driver/gpio.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include <stdint.h>

uint8_t level = 0;
void blink() {
  gpio_set_level(GPIO_NUM_2, level);
  level = !level;
};

void blink_fatal() {
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(GPIO_NUM_2, 1);
    vTaskDelay(pdMS_TO_TICKS(400));
    gpio_set_level(GPIO_NUM_2, 0);

    for (uint8_t t = 0; t < 4; t++) {
      vTaskDelay(pdMS_TO_TICKS(50));
      gpio_set_level(GPIO_NUM_2, 1);
      vTaskDelay(pdMS_TO_TICKS(50));
      gpio_set_level(GPIO_NUM_2, 0);
    }
  }
};

void blink_once() {
  gpio_set_level(GPIO_NUM_2, 1);
  vTaskDelay(pdMS_TO_TICKS(300));
  gpio_set_level(GPIO_NUM_2, 0);
  vTaskDelay(pdMS_TO_TICKS(100));
};
