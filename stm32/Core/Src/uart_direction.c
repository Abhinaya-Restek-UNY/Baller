#include "uart_direction.h"
#include "stm32f4xx_hal_uart.h"
#include <string.h>

static uint8_t uart_buf[sizeof(direction_packet)];
static uint8_t count = 0;
static int16_t x = 0;
static int16_t y = 0;
void (*new_data_callback)(int16_t x, int16_t y) = NULL;

void new_data_arrived() {
  direction_packet *pkt = (direction_packet *)(uart_buf);
  x = pkt->dir_x;
  y = pkt->dir_y;
  if (new_data_callback != NULL) {
    new_data_callback(x, y);
  }
}

int8_t find_newline(uint8_t *buf, uint8_t len) {
  for (uint8_t i = 0; i < len; i++) {
    if (buf[i] == '\n') {
      return i;
    }
  }

  return -1;
};

void process_packet_new_beginning(uint8_t *packet, uint16_t packet_size) {
  int8_t newline = find_newline(packet, packet_size);
  count = 0;
  if (packet_size == sizeof(direction_packet)) {
    memcpy(uart_buf, packet, sizeof(direction_packet));
    return new_data_arrived();
  } else if () {
    count += packet_size - newline;
  };
};

void uart_direction_poll(uint8_t *packet, uint16_t packet_size) {
  int8_t newline = find_newline(packet, packet_size);
  if (packet_size > sizeof(direction_packet)) {
    if (newline < 0) {
      return;
    }
    process_packet_new_beginning(&packet[newline], packet_size - newline);
  }
}
