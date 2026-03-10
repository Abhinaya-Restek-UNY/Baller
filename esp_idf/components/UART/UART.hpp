#pragma once

#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <functional>
#include <memory>
#include <stdint.h>

class UART {
public:
  // Define the type for our receive callback
  using uart_on_read_cb_t = std::function<void(uint8_t *data, uint16_t size)>;

  /**
   * @brief Constructor
   * @param read_buf_size Size of the internal read buffer
   * @param uart_num Hardware UART port (Defaults to UART_NUM_0 for USB console)
   */
  UART(uint16_t read_buf_size, uart_port_t uart_num = UART_NUM_0);

  // Destructor
  ~UART();

  /**
   * @brief Initializes the UART driver and starts the background interrupt task
   * @return true if successful, false otherwise
   */
  bool begin(int baud_rate = 115200, int tx_pin = UART_PIN_NO_CHANGE,
             int rx_pin = UART_PIN_NO_CHANGE,
             int request_to_send_pin = UART_PIN_NO_CHANGE,
             int clear_to_send_pin = UART_PIN_NO_CHANGE);

  /**
   * @brief Strictly blocking write of raw binary data
   * @param data Pointer to the binary data
   * @param size Number of bytes to send
   */
  void write(const uint8_t *data, uint16_t size);

  /**
   * @brief Register a callback function to be fired when data is received
   * @param cb The function/lambda to execute
   */
  void setReceiveCallback(uart_on_read_cb_t cb);

private:
  uint16_t read_buf_size;
  uart_port_t uart_num;
  QueueHandle_t uart_queue;
  TaskHandle_t task_handle;
  uart_on_read_cb_t rx_callback;

  std::unique_ptr<uint8_t[]> read_buffer;

  // Static wrapper required because FreeRTOS is C-based and doesn't know about
  // "this"
  static void taskWrapper(void *arg);

  // The actual event loop running inside the object instance
  void processEvents();
};
