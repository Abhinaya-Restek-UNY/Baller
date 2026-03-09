
#include "MPU6050.h"
#include "USBUart.hpp"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "serial_hub.h"
#include <driver/i2c.h>
#include <esp_err.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define MPU_INT_PIN GPIO_NUM_19
USBUart serialPort(400);
serial_hub_handle_t serial_hub;

uint8_t level = 0;
void blink() {
  gpio_set_level(GPIO_NUM_2, level);
  level = !level;
};

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

uint32_t prev;
uint8_t *fifo_buffer;

Quaternion quat;

VectorInt16 accel_raw;

VectorFloat gravity;
VectorInt16 accel_real;

void mpu_task(void *pvParameter) {
  MPU6050 *mpu = (MPU6050 *)pvParameter;

  float quat_f[4];
  uint8_t count = 0;
  while (1) {

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    mpu->dmpGetCurrentFIFOPacket(fifo_buffer);
    mpu->dmpGetQuaternion(&quat, fifo_buffer);
    mpu->dmpGetAccel(&accel_raw, fifo_buffer);
    mpu->dmpGetGravity(&gravity, &quat);
    mpu->dmpGetLinearAccel(&accel_real, &accel_raw, &gravity);
    quat_f[0] = quat.w;
    quat_f[1] = quat.x;
    quat_f[2] = quat.y;
    quat_f[3] = quat.z;

    serial_hub_write_topic(&serial_hub, 1, (uint8_t *)quat_f, sizeof(quat_f));

    if (count == 0) {
      blink();
    }
    count = (count + 1) % 30;
  }
}
//
void setup_interrupt(MPU6050 *mpu) {

  gpio_config_t io_conf{
      .pin_bit_mask = (1ULL << MPU_INT_PIN),
      .mode = GPIO_MODE_INPUT,
      .pull_up_en =
          GPIO_PULLUP_DISABLE, // MPU6050 INT is usually Push-Pull active high
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_POSEDGE, // Trigger on Rising Edge (0 -> 1)
  };

  xTaskCreate(mpu_task, "mpu_task", 4096, mpu, 10, &mpuTaskHandle);

  gpio_config(&io_conf);

  gpio_install_isr_service(ESP_INTR_FLAG_IRAM);

  gpio_isr_handler_add(MPU_INT_PIN, gpio_isr_handler, NULL);
}

void write_cb(USBUart *ser_port, uint8_t *data, fsize_t size) {
  ser_port->write(data, size);
}

extern "C" void app_main(void) {

  gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);

  i2c_master_bus_handle_t bus_hande;
  i2c_master_bus_config_t bus_conf = {
      .i2c_port = I2C_NUM_0,
      .sda_io_num = GPIO_NUM_21,
      .scl_io_num = GPIO_NUM_22,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7,
      .flags = {.enable_internal_pullup = true}};

  ESP_ERROR_CHECK(i2c_new_master_bus(&bus_conf, &bus_hande));

  MPU6050 mpu = MPU6050(bus_hande);

  mpu.initialize();

  uint8_t devStatus = mpu.dmpInitialize();
  mpu.setXAccelOffset(-2407);
  mpu.setYAccelOffset(1065);
  mpu.setZAccelOffset(1681);
  mpu.setXGyroOffset(33);
  mpu.setYGyroOffset(-2);
  mpu.setZGyroOffset(-17);

  if (devStatus == 0) {

    mpu.setDMPEnabled(true);
    fifo_buffer = new uint8_t[mpu.dmpGetFIFOPacketSize()];

    mpu.getIntStatus();
    mpu.resetFIFO();

    serialPort.begin();
    serial_hub_initialize(&serial_hub, (write_cb_t)write_cb, &serialPort);
    serial_hub_reserve_memory(&serial_hub, sizeof(float) * 5);
    setup_interrupt(&mpu);
  } else {

    bool level = 0;
    while (1) {
      gpio_set_level(GPIO_NUM_2, level);
      level = level == 0 ? 1 : 0;
      vTaskDelay(pdMS_TO_TICKS(500));
    }
  }
  while (1) {
    vTaskDelay(10);
  }
}
