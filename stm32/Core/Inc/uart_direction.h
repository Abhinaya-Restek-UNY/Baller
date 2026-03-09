typedef struct {
  int16_t dir_x;
  int16_t dir_y;
  char end_of_transmission;

} direction_packet;

void uart_direction_poll(uint8_t *packet, uint16_t packet_size);
