#pragma once

#include "boost/asio.hpp"
#include "rclcpp/rclcpp.hpp"

#include "serial_hub.h"
#include <cstdint>
#include <string>
#include <thread>

class AsioSerialHub {
public:
  AsioSerialHub(std::string device, uint32_t baud_rate, fsize_t max_message,
                rclcpp::Logger logger)
      : serial_io_port(this->serial_io_context),
        work_guard_(boost::asio::make_work_guard(this->serial_io_context)),
        logger(logger), device(device), baud_rate(baud_rate) {

    serial_hub_initialize(&this->hub, (write_cb_t)this->write_to_serial, this);
    serial_hub_reserve_memory(&this->hub, max_message);

    asio_thread = std::thread([this]() { this->serial_io_context.run(); });

    this->asio_read_buf =
        (std::make_unique<uint8_t[]>(this->hub.__read_buf_size * 2));
  };

  ~AsioSerialHub() {
    work_guard_.reset();
    serial_io_context.stop();
    if (asio_thread.joinable()) {
      asio_thread.join();
    }
  }

  bool is_open() { return serial_io_port.is_open(); };

  int8_t connect() {
    if (this->serial_io_port.is_open()) {
      return -1;
    }
    RCLCPP_INFO(this->logger, "Connecting to %s with %d baud rate...",
                this->device.c_str(), this->baud_rate);
    try {

      this->serial_io_port.open(this->device);

      this->serial_io_port.set_option(
          boost::asio::serial_port_base::baud_rate(this->baud_rate));

      this->serial_io_port.set_option(
          boost::asio::serial_port_base::character_size(8));

      this->serial_io_port.set_option(boost::asio::serial_port_base::parity(
          boost::asio::serial_port_base::parity::none));
      this->serial_io_port.set_option(boost::asio::serial_port_base::stop_bits(
          boost::asio::serial_port_base::stop_bits::one));
      this->serial_io_port.set_option(
          boost::asio::serial_port_base::flow_control(
              boost::asio::serial_port_base::flow_control::none));

      RCLCPP_INFO(this->logger, "Successfully connected to serial.");
      receive_loop();
      return 0;
    } catch (const boost::system::system_error &e) {
      RCLCPP_WARN(this->logger,
                  "%s\nFailed to connect to serial. will retry in 2 seconds...",
                  e.what());

      return -1;
    }
  };

  template <typename T, typename ctxType>
  int8_t attach(uint8_t id, void *ctx,
                void (*callback)(ctxType *ctx, T *data, fsize_t size)) {
    return serial_hub_attach_topic(&this->hub, id, sizeof(T), ctx,
                                   (on_receive_cb_t)callback);
  }

  template <typename T> void write(uint8_t id, T *packet) {
    serial_hub_write_topic(&this->hub, id, (uint8_t *)packet, sizeof(T));
  }

private:
  // WARN: Make sure this is on top.
  boost::asio::io_context serial_io_context;

  boost::asio::serial_port serial_io_port;

  boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
      work_guard_;

  std::thread asio_thread;
  std::unique_ptr<uint8_t[]> asio_read_buf;

  serial_hub_handle_t hub;

  rclcpp::Logger logger;

  std::string device;
  uint32_t baud_rate;

  static void write_to_serial(AsioSerialHub *_this, uint8_t *data,
                              fsize_t size) {
    if (_this->serial_io_port.is_open()) {
      boost::asio::write(_this->serial_io_port,
                         boost::asio::buffer(data, size));
      return;
    }

    RCLCPP_WARN_STREAM(_this->logger, "Serial port is not open. dropping "
                                          << size << " byte packet.");
  };

  void receive_loop() {
    this->serial_io_port.async_read_some(
        boost::asio::buffer(this->asio_read_buf.get(),
                            this->hub.__read_buf_size * 2),
        [this](const boost::system::error_code &error,
               std::size_t bytes_transferred) {
          if (!error) {
            serial_hub_on_read(&this->hub, this->asio_read_buf.get(),
                               bytes_transferred);

            receive_loop();

          } else {
            RCLCPP_ERROR(this->logger, "Serial read error: %s",
                         error.message().c_str());

            boost::system::error_code ec;
            this->serial_io_port.close(ec);

            RCLCPP_INFO(this->logger,
                        "Port closed. Reconnection timer will take over.");
          }
        });
  };
};
