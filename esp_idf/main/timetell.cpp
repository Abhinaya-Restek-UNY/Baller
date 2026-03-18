#include "timetell.hpp"
#include "blink.hpp"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "serial_hub.h"

uint64_t get_time_micros() {
  return static_cast<uint64_t>(esp_timer_get_time());
};

TaskHandle_t timetell_task;
void vPeriodicTask(void *pvParameters) {
  serial_hub_handle_t *serial_hub = (serial_hub_handle_t *)pvParameters;

  packet_time timepack = {
      .delta = 0, .timestamp = get_time_micros(), .unit_index = 1};

  serial_hub_write_topic(serial_hub, TIME_PACKET_ID, (uint8_t *)&timepack,
                         sizeof(timepack));

  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(5000);
  xTaskDelayUntil(&xLastWakeTime, xFrequency);

  while (true) {
    uint64_t current = get_time_micros();
    timepack.delta = current - timepack.timestamp;
    timepack.timestamp = current;

    serial_hub_write_topic(serial_hub, TIME_PACKET_ID, (uint8_t *)&timepack,
                           sizeof(packet_time));

    xTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

void start_telling_time(serial_hub_handle_t *hub) {
  xTaskCreate(vPeriodicTask, "Time telling", 2048, hub, 10, &timetell_task);
};
// uint32_t get_time_ms();
