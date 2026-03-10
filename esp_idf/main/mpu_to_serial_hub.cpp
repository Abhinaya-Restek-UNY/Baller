#include "mpu_to_serial_hub.hpp"
#include "blink.hpp"
#include "driver/i2c_master.h"
#include "freertos/projdefs.h"
#include "serial_hub.h"
#include <memory>

static void IRAM_ATTR gpio_isr_handler(void *arg) {

  // Notify the task that an interrupt happened.
  // We use "FromISR" version because we are inside an interrupt.
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(mpuTaskHandle, &xHigherPriorityTaskWoken);

  // If the task we woke up is high priority, switch to it immediately.
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void __mpu_task(void *pvParameter) {
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

std::unique_ptr<MPU6050> mpu;

void write_cb(UART *ser_port, uint8_t *data, fsize_t size) {
  ser_port->write(data, size);
}
int8_t setup_mpu6050_interrupt(UART *io, gpio_num_t interrupt_pin) {
  // TODO: serial_hub shall not be here.
  serial_hub_initialize(&serial_hub, (write_cb_t)write_cb, io);
  serial_hub_reserve_memory(&serial_hub, sizeof(quat));

  gpio_config_t io_conf{
      .pin_bit_mask = (1ULL << interrupt_pin),
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_POSEDGE,
  };

  mpu->getIntStatus();
  mpu->resetFIFO();

  if (xTaskCreate(__mpu_task, "mpu_task", 4096, mpu.get(), 10,
                  &mpuTaskHandle) != pdPASS) {
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
