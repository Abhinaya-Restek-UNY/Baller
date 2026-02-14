#include "MPU6050.hpp"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"

#define MPU_INT_PIN GPIO_NUM_15

#define MAIN_TAG "MAIN"
static TaskHandle_t mpuTaskHandle = NULL;

// 1. THE INTERRUPT HANDLER (Keep this FAST!)
// IRAM_ATTR forces this code into RAM so it runs super fast.
static void IRAM_ATTR gpio_isr_handler(void *arg) {

  // Notify the task that an interrupt happened.
  // We use "FromISR" version because we are inside an interrupt.
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(mpuTaskHandle, &xHigherPriorityTaskWoken);

  // If the task we woke up is high priority, switch to it immediately.
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// 2. THE WORKER TASK (This does the heavy lifting)
void mpu_task(void *pvParameter) {
  while (1) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // --- WE ARE AWAKE NOW ---
    ((MPU6050 *)pvParameter)->update_sample();
  }
}

extern "C" void app_main(void) {

  i2c_master_bus_handle_t bus_hande;
  i2c_master_bus_config_t bus_conf = {
      .i2c_port = I2C_NUM_0,
      .sda_io_num = GPIO_NUM_21,
      .scl_io_num = GPIO_NUM_22,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7,
      .flags = {.enable_internal_pullup = true}};
  ESP_ERROR_CHECK(i2c_new_master_bus(&bus_conf, &bus_hande));

  MPU6050 mpu(bus_hande);
  xTaskCreate(mpu_task, "mpu_task", 4096, &mpu, 10, &mpuTaskHandle);

  gpio_config_t io_conf{
      .pin_bit_mask = (1ULL << MPU_INT_PIN),
      .mode = GPIO_MODE_INPUT,
      .pull_up_en =
          GPIO_PULLUP_DISABLE, // MPU6050 INT is usually Push-Pull active high
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_POSEDGE, // Trigger on Rising Edge (0 -> 1)
  };

  mpu.init();

  gpio_config(&io_conf);
  gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
  gpio_isr_handler_add(MPU_INT_PIN, gpio_isr_handler, NULL);

  vTaskDelay(pdMS_TO_TICKS(2000));

  mpu.calibrate();

  mpu.start();

  while (true) {
    vTaskDelay(300);
  }
}
