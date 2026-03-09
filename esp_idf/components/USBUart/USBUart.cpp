#include "USBUart.hpp"
#include "esp_log.h"
#include <cstring>

static const char *TAG = "USBUart_OOP";

USBUart::USBUart(uint16_t read_buf_size, uart_port_t uart_num)
    : m_read_buf_size(read_buf_size), m_uart_num(uart_num),
      m_uart_queue(nullptr), m_task_handle(nullptr), m_rx_callback(nullptr) {}

USBUart::~USBUart() {
  if (m_task_handle) {
    vTaskDelete(m_task_handle);
  }
  uart_driver_delete(m_uart_num);
}

bool USBUart::begin() {
  uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  };

  uart_param_config(m_uart_num, &uart_config);
  uart_set_pin(m_uart_num, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,
               UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  // Delete any pre-existing driver to avoid conflict
  uart_driver_delete(m_uart_num);

  // Install driver and allocate queue
  esp_err_t err =
      uart_driver_install(m_uart_num, m_read_buf_size * 2, m_read_buf_size * 2,
                          20, &m_uart_queue, 0);

  if (err != ESP_OK || m_uart_queue == nullptr) {
    ESP_LOGE(TAG, "Failed to install UART driver or allocate queue!");
    return false;
  }

  // Create the task, passing 'this' as the argument so FreeRTOS knows which
  // object to talk to
  BaseType_t task_created = xTaskCreate(taskWrapper, "uart_event_task", 4096,
                                        this, 12, &m_task_handle);

  if (task_created != pdPASS) {
    ESP_LOGE(TAG, "Failed to create UART task!");
    return false;
  }

  ESP_LOGI(TAG, "UART OOP Initialized Successfully.");
  return true;
}

void USBUart::write(const uint8_t *data, uint16_t size) {
  uart_write_bytes(m_uart_num, (const void *)data, size);

  // As requested earlier, make it strictly block until completely sent
  // uart_wait_tx_done(m_uart_num, portMAX_DELAY);
}

void USBUart::setRxCallback(RxCallback cb) { m_rx_callback = cb; }

// --- FREE RTOS TASK WRAPPER ---
void USBUart::taskWrapper(void *arg) {
  // Cast the void pointer back to our C++ class instance
  USBUart *instance = static_cast<USBUart *>(arg);

  // Call the actual member function
  instance->processEvents();
}

void USBUart::processEvents() {
  uart_event_t event;

  // Allocate the dynamic buffer cleanly in C++
  uint8_t *dtmp = new uint8_t[m_read_buf_size];

  for (;;) {
    // Wait for an interrupt event
    if (xQueueReceive(m_uart_queue, (void *)&event, portMAX_DELAY)) {
      switch (event.type) {
      case UART_DATA:
        uart_read_bytes(m_uart_num, dtmp, event.size, portMAX_DELAY);

        // Fire the callback if the user set one in main.cpp
        if (m_rx_callback) {
          m_rx_callback(dtmp, event.size);
        }
        break;

      case UART_FIFO_OVF:
      case UART_BUFFER_FULL:
        uart_flush_input(m_uart_num);
        xQueueReset(m_uart_queue);
        break;

      default:
        break;
      }
    }
  }

  delete[] dtmp;
  vTaskDelete(nullptr);
}
