#include "serial_hub.h"
#include <stdint.h>
void setup_time_tell_task();

#define TIME_PACKET_ID 3

typedef struct __attribute__((packed)) {
  uint64_t delta;
  uint64_t timestamp;
  uint8_t unit_index;
} packet_time;

uint64_t get_time_micros();

void vPeriodicTask(void *pvParameters);

void start_telling_time(serial_hub_handle_t *hub);
// uint32_t get_time_ms();
