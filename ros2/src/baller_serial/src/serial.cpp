#include "baller_interfaces/srv/motor_parameter.hpp"
#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <thread>

#include "boost/asio.hpp"
#include "serial_hub.h"

#define ENCODER_PACKET_ID 1
#define MPU_PACKET_ID 2
#define TIME_PACKET_ID 3
#define MOTOR_PACKET_ID 4
using namespace std::chrono_literals;
typedef int16_t motor_direction_t;

typedef struct __attribute__((packed)) {
  motor_direction_t front_right;
  motor_direction_t front_left;
  motor_direction_t back_right;
  motor_direction_t back_left;
} packet_motor;

typedef struct __attribute__((packed)) {
  uint8_t unit_index;
  uint64_t timestamp;
  uint64_t delta;
} packet_time;

typedef struct __attribute__((packed)) {
  int32_t revolutionA;
  int32_t revolutionB;
  int32_t revolutionC;
  uint64_t timestamp;
} packet_encoder;

typedef struct __attribute__((packed)) {
  float q_w;
  float q_x;
  float q_y;
  float q_z;
  int a_x;
  int a_y;
  int a_z;
  uint64_t timestamp;
} packet_mpu;

class BallerSerial : public rclcpp::Node {
public:
  BallerSerial()
      : Node("baller_serial"), serial_io_port(this->serial_io_context),
        work_guard_(boost::asio::make_work_guard(this->serial_io_context)) {

    this->declare_parameter<std::string>("serial_port", "/dev/ttyUSB0");
    this->declare_parameter<int>("baud_rate", 115200);

    this->set_motor_service =
        this->create_service<baller_interfaces::srv::MotorParameter>(
            "set_motor",
            std::bind(&BallerSerial::handle_set_motor_request, this,
                      std::placeholders::_1, std::placeholders::_2));

    this->serial_io_port_reconnect_timer = this->create_wall_timer(
        3s, std::bind(&BallerSerial::connect_to_serial, this));

    serial_hub_initialize(&this->hub, this->write_to_serial, this);
    serial_hub_reserve_memory(&this->hub, sizeof(packet_mpu));

    serial_hub_attach_topic(&this->hub, ENCODER_PACKET_ID,
                            sizeof(packet_encoder), this,
                            (on_receive_cb_t)on_receive_encoder);

    serial_hub_attach_topic(&this->hub, MPU_PACKET_ID, sizeof(packet_mpu), this,
                            (on_receive_cb_t)on_receive_mpu);

    serial_hub_attach_topic(&this->hub, TIME_PACKET_ID, sizeof(packet_time),
                            this, (on_receive_cb_t)on_receive_time);

    asio_thread = std::thread([this]() { this->serial_io_context.run(); });

    this->asio_read_buf =
        (std::make_unique<uint8_t[]>(this->hub.__read_buf_size * 2));
  }

  ~BallerSerial() {
    work_guard_.reset();
    serial_io_context.stop();
    if (asio_thread.joinable()) {
      asio_thread.join();
    }
  }

private:
  boost::asio::io_context serial_io_context;
  boost::asio::serial_port serial_io_port;

  boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
      work_guard_;

  std::thread asio_thread;

  rclcpp::TimerBase::SharedPtr serial_io_port_reconnect_timer;

  std::unique_ptr<uint8_t[]> asio_read_buf;

  serial_hub_handle_t hub;

  rclcpp::Service<baller_interfaces::srv::MotorParameter>::SharedPtr
      set_motor_service;

  void handle_set_motor_request(
      const std::shared_ptr<baller_interfaces::srv::MotorParameter::Request>
          request,
      std::shared_ptr<baller_interfaces::srv::MotorParameter::Response>
          response) {
    if (!this->serial_io_port.is_open()) {
      response->success = false;
      return; // Stop right here
    }
    packet_motor cmd = {.front_right = request->front_right,
                        .front_left = request->front_left,
                        .back_right = request->back_right,
                        .back_left = request->back_left};
    serial_hub_write_topic(&this->hub, MOTOR_PACKET_ID, (uint8_t *)&cmd,
                           sizeof(packet_motor));
    response->success = true;
  }
  static void write_to_serial(void *ctx, uint8_t *data, fsize_t size) {
    BallerSerial *_this = (BallerSerial *)ctx;
    if (_this->serial_io_port.is_open()) {
      _this->serial_io_port.write_some(boost::asio::buffer(data, size));
      return;
    }

    RCLCPP_WARN_STREAM(_this->get_logger(), "Serial port is not open. dropping "
                                                << size << " byte packet.");
  };

  static void on_receive_mpu(BallerSerial *_this, packet_mpu *packet,
                             fsize_t size) {
    RCLCPP_INFO(
        _this->get_logger(),
        "Received mpu: accel(%4d, %4d, %4d) quatern(%.2f, %.2f, %.2f, %.2f)",
        packet->a_x, packet->a_y, packet->a_z, packet->q_w, packet->q_x,
        packet->q_y, packet->q_z);
    // TODO: Integrate with ukf_localization_node and packet_time
  };

  static void on_receive_encoder(BallerSerial *_this, packet_encoder *packet,
                                 fsize_t size) {

    RCLCPP_INFO(_this->get_logger(), "Received encoder: %d %d %d",
                packet->revolutionA, packet->revolutionB, packet->revolutionC);
    // TODO: Integrate with ukf_localization_node and packet_time
  }

  static void on_receive_time(BallerSerial *_this, packet_time *packet,
                              fsize_t size) {
    RCLCPP_INFO(_this->get_logger(),
                "Received time: index(%d) timestamp(%lu) delta(%lu) ",
                packet->unit_index, packet->timestamp, packet->delta);

    // TODO: Create local time syncing mechanism to integrate with
    // ukf_localization_node
  }
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
            RCLCPP_ERROR(this->get_logger(), "Serial read error: %s",
                         error.message().c_str());

            boost::system::error_code ec;
            this->serial_io_port.close(ec);

            RCLCPP_INFO(this->get_logger(),
                        "Port closed. Reconnection timer will take over.");
          }
        });
  };

  int8_t connect_to_serial() {
    if (this->serial_io_port.is_open()) {
      return 0;
    }
    std::string port_name = this->get_parameter("serial_port").as_string();
    uint32_t baud_rate = this->get_parameter("baud_rate").as_int();
    RCLCPP_INFO(this->get_logger(), "Connecting to %s with %d baud rate...",
                port_name.c_str(), baud_rate);
    try {

      this->serial_io_port.open(port_name);

      this->serial_io_port.set_option(
          boost::asio::serial_port_base::baud_rate(baud_rate));

      this->serial_io_port.set_option(
          boost::asio::serial_port_base::character_size(8));

      this->serial_io_port.set_option(boost::asio::serial_port_base::parity(
          boost::asio::serial_port_base::parity::none));
      this->serial_io_port.set_option(boost::asio::serial_port_base::stop_bits(
          boost::asio::serial_port_base::stop_bits::one));
      this->serial_io_port.set_option(
          boost::asio::serial_port_base::flow_control(
              boost::asio::serial_port_base::flow_control::none));

      RCLCPP_INFO(this->get_logger(), "Successfully connected to serial.");
      receive_loop();
      return 0;
    } catch (const boost::system::system_error &e) {
      RCLCPP_WARN(this->get_logger(),
                  "%s\nFailed to connect to serial. will retry in 2 seconds...",
                  e.what());

      return -1;
    }
  };
};

int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<BallerSerial>());
  rclcpp::shutdown();
  return 0;
}
