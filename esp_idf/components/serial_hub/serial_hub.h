#ifndef SERIAL_HUB_H
#define SERIAL_HUB_H

#ifdef __cplusplus
extern "C" {
#endif
#include "stdint.h"

#define SERIAL_HUB_TOPIC_MAX 8

typedef uint16_t fsize_t;

typedef uint16_t fssize_t;

typedef void (*on_receive_cb_t)(uint8_t *data, fsize_t size);

typedef void (*write_cb_t)(void *ctx, uint8_t *data, fsize_t size);

typedef struct {
  uint8_t id;
  fsize_t size;
  uint8_t data[]; // Flexible array member (must be last)
} __attribute__((packed)) serial_hub_raw_packet;

typedef struct {
  fsize_t max_length;
  fsize_t expected_length;
  int8_t state;
  on_receive_cb_t callback;
} serial_hub_topic_t;

typedef struct {
  int8_t state;
  uint8_t current_topic;

  uint8_t *__read_buf;
  uint8_t *__write_buf;
  uint8_t *__cobsr_tmp_buf;
  fsize_t __read_buf_size;
  fsize_t __write_buf_size;
  fsize_t __cobsr_tmp_buf_size;

  fsize_t __expected_length;
  fsize_t __real_max_length;
  void *__write_cb_context;

  uint16_t __count;
  write_cb_t __write_cb;
  serial_hub_topic_t topics[SERIAL_HUB_TOPIC_MAX];
} serial_hub_handle_t;

#define SERIAL_HUB_ERR_INVALID_SIZE -1
#define SERIAL_HUB_ERR_INVALID_STATE -1
#define SERIAL_HUB_ERR_OCCUPIED -2
#define SERIAL_HUB_ERR_NOT_INITIALIZED -3
#define SERIAL_HUB_ERR_FATAL -4
#define SERIAL_HUB_ERR_OUT_OF_BOUND -5

#define SERIAL_HUB_STATE_INITIALIZED 1
#define SERIAL_HUB_STATE_UNITIALIZED 2
#define SERIAL_HUB_STATE_DEINITIALIZED -1

#define SERIAL_HUB_READ_STATE_HIT_FIRST_ZERO 3
#define SERIAL_HUB_READ_STATE_WAITING_FOR_LENGTH 4
#define SERIAL_HUB_READ_STATE_WAITING_FOR_LENGTH_MSB 5
#define SERIAL_HUB_READ_STATE_READING 6
#define SERIAL_HUB_READ_STATE_WAITING_FOR_ID 8
#define SERIAL_HUB_READ_STATE_EMPTY 7

#define SERIAL_HUB_OK 0

void serial_hub_on_read(serial_hub_handle_t *handle, uint8_t *data,
                        fsize_t size);

int8_t serial_hub_attach_topic(serial_hub_handle_t *handle, uint8_t id,
                               fsize_t size, on_receive_cb_t callback);

int8_t serial_hub_dettach_topic(serial_hub_handle_t *handle, uint8_t id,
                                on_receive_cb_t callback);

int8_t serial_hub_write_topic(serial_hub_handle_t *handle, uint8_t id,
                              uint8_t *data, fsize_t size);

void serial_hub_get_err(char *str, fsize_t size);

int8_t serial_hub_initialize(serial_hub_handle_t *handle, write_cb_t write_cb,
                             void *ctx);

fsize_t serial_hub_reserve_memory(serial_hub_handle_t *handle,
                                  fsize_t max_message_length);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // SERIAL_HUB_H
