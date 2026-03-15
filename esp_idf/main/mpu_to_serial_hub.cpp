#include "mpu_to_serial_hub.hpp"
#include "blink.hpp"
#include "driver/i2c_master.h"
#include "freertos/projdefs.h"
#include "serial_hub.h"
#include "timetell.hpp"
#include <memory>

static void IRAM_ATTR gpio_isr_handler(void *arg) {

  // Notify the task that an interrupt happened.
  // We use "FromISR" version because we are inside an interrupt.
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(mpuTaskHandle, &xHigherPriorityTaskWoken);

  // If the task we woke up is high priority, switch to it immediately.
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

struct __mpu_task_param {
  MPU6050 *mpu;
  serial_hub_handle_t *hub;
};

void __mpu_task(void *pvParameter) {
  MPU6050 *mpu = ((__mpu_task_param *)pvParameter)->mpu;
  serial_hub_handle_t *serial_hub = ((__mpu_task_param *)pvParameter)->hub;

  packet_mpu packet = {0, 0, 0, 0, 0, 0, 0};
  while (1) {

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    packet.timestamp = get_time_micros();

    mpu->dmpGetCurrentFIFOPacket(fifo_buffer);
    mpu->dmpGetQuaternion(&quat, fifo_buffer);
    mpu->dmpGetAccel(&accel_raw, fifo_buffer);
    mpu->dmpGetGravity(&gravity, &quat);
    mpu->dmpGetLinearAccel(&accel_real, &accel_raw, &gravity);
    packet.q_w = quat.w;
    packet.q_x = quat.x;
    packet.q_y = quat.y;
    packet.q_z = quat.z;

    packet.a_x = accel_real.x;
    packet.a_y = accel_real.y;
    packet.a_z = accel_real.z;

    serial_hub_write_topic(serial_hub, MPU_PACKET_ID, (uint8_t *)&packet,
                           sizeof(packet_mpu));
    blink();
  }

  delete[] (__mpu_task_param *)pvParameter;
}

std::unique_ptr<MPU6050> mpu;

int8_t setup_mpu6050_interrupt(serial_hub_handle_t *serial_hub,
                               gpio_num_t interrupt_pin) {
  // TODO: serial_hub shall not be here.

  gpio_config_t io_conf{
      .pin_bit_mask = (1ULL << interrupt_pin),
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_POSEDGE,
  };

  mpu->getIntStatus();
  mpu->resetFIFO();

  __mpu_task_param *_p =
      new __mpu_task_param{.mpu = mpu.get(), .hub = serial_hub};
  if (xTaskCreate(__mpu_task, "mpu_task", 4096, _p, 10, &mpuTaskHandle) !=
      pdPASS) {
    return -1;
  }

  if (gpio_config(&io_conf) != ESP_OK) {
    return -1;
  }

  if (gpio_install_isr_service(ESP_INTR_FLAG_IRAM) != ESP_OK) {
    return -1;
  }

  if (gpio_isr_handler_add(interrupt_pin, gpio_isr_handler, NULL) != ESP_OK) {
    return -1;
  }

  return 0;
}

int8_t setup_mpu6050(gpio_num_t SCL, gpio_num_t SDA) {

  i2c_master_bus_handle_t bus_hande;
  i2c_master_bus_config_t bus_conf = {
      .i2c_port = I2C_NUM_0,
      .sda_io_num = SDA,
      .scl_io_num = SCL,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7,
      .flags = {.enable_internal_pullup = true}};

  ESP_ERROR_CHECK(i2c_new_master_bus(&bus_conf, &bus_hande));

  mpu = std::make_unique<MPU6050>(bus_hande);

  mpu->initialize();

  uint8_t devStatus = mpu->dmpInitialize();
  mpu->setXAccelOffset(-2407);
  mpu->setYAccelOffset(1065);
  mpu->setZAccelOffset(1681);
  mpu->setXGyroOffset(33);
  mpu->setYGyroOffset(-2);
  mpu->setZGyroOffset(-17);

  if (devStatus == 0) {

    mpu->setDMPEnabled(true);
    fifo_buffer = new uint8_t[mpu->dmpGetFIFOPacketSize()];

    mpu->getIntStatus();
    mpu->resetFIFO();
  }
  return devStatus;
};
