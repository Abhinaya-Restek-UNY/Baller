#include "UART.hpp"
#include <cstring>
#include <memory>

UART::UART(uint16_t read_buf_size, uart_port_t uart_num)
    : read_buf_size(read_buf_size), uart_num(uart_num), uart_queue(nullptr),
      task_handle(nullptr), rx_callback(nullptr),
      read_buffer(std::make_unique<uint8_t[]>(this->read_buf_size)) {}

UART::~UART() {
  if (task_handle) {
    vTaskDelete(task_handle);
  }
  uart_driver_delete(uart_num);
}

bool UART::begin(int baud_rate, int tx_pin, int rx_pin, int request_to_send_pin,
                 int clear_to_send_pin) {
  uart_config_t uart_config = {
      .baud_rate = baud_rate,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  };

  uart_param_config(this->uart_num, &uart_config);
  uart_set_pin(this->uart_num, tx_pin, rx_pin, request_to_send_pin,
               clear_to_send_pin);

  uart_driver_delete(this->uart_num);

  esp_err_t err =
      uart_driver_install(this->uart_num, this->read_buf_size * 2,
                          this->read_buf_size * 2, 20, &this->uart_queue, 0);

  if (err != ESP_OK || this->uart_queue == nullptr) {
    return false;
  }

  BaseType_t task_created = xTaskCreate(taskWrapper, "uart_event_task", 4096,
                                        this, 12, &this->task_handle);

  if (task_created != pdPASS) {
    return false;
  }

  return true;
}

void UART::write(const uint8_t *data, uint16_t size) {
  uart_write_bytes(this->uart_num, (const void *)data, size);
}

void UART::setReceiveCallback(uart_on_read_cb_t cb) { rx_callback = cb; }

void UART::taskWrapper(void *arg) {
  UART *instance = static_cast<UART *>(arg);

  instance->processEvents();
}

void UART::processEvents() {
  uart_event_t event;

  for (;;) {
    if (xQueueReceive(this->uart_queue, (void *)&event, portMAX_DELAY)) {
      switch (event.type) {
      case UART_DATA:
        uart_read_bytes(this->uart_num, this->read_buffer.get(), event.size,
                        portMAX_DELAY);

        if (rx_callback) {
          rx_callback(this->read_buffer.get(), event.size);
        }
        break;

      case UART_FIFO_OVF:
      case UART_BUFFER_FULL:
        uart_flush_input(this->uart_num);
        xQueueReset(uart_queue);
        break;

      default:
        break;
      }
    }
  }

  vTaskDelete(nullptr);
}
