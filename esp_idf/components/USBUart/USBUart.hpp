#pragma once

#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <functional>
#include <stdint.h>

class USBUart {
public:
  // Define the type for our receive callback
  using RxCallback = std::function<void(uint8_t *data, uint16_t size)>;

  /**
   * @brief Constructor
   * @param read_buf_size Size of the internal read buffer
   * @param uart_num Hardware UART port (Defaults to UART_NUM_0 for USB console)
   */
  USBUart(uint16_t read_buf_size, uart_port_t uart_num = UART_NUM_0);

  // Destructor
  ~USBUart();

  /**
   * @brief Initializes the UART driver and starts the background interrupt task
   * @return true if successful, false otherwise
   */
  bool begin();

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
  void setRxCallback(RxCallback cb);

private:
  uint16_t m_read_buf_size;
  uart_port_t m_uart_num;
  QueueHandle_t m_uart_queue;
  TaskHandle_t m_task_handle;
  RxCallback m_rx_callback;

  // Static wrapper required because FreeRTOS is C-based and doesn't know about
  // "this"
  static void taskWrapper(void *arg);

  // The actual event loop running inside the object instance
  void processEvents();
};
