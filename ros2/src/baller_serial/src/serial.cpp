#include "AsioSerialHub.hpp"
#include "Timesync.hpp"
#include "baller_interfaces/msg/motor_parameter.hpp"
#include "geometry_msgs/msg/pose_with_covariance_stamped.hpp"
#include <rclcpp/rclcpp.hpp>

#include "boost/asio.hpp"
#include "packet_types.h"

#include "RobotLocalizationBridge.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "sensor_msgs/msg/imu.hpp"

using namespace std::chrono_literals;

class BallerSerial : public rclcpp::Node {
public:
  BallerSerial()
      : Node("baller_serial"),
        robotLocalizationBridge(
            this->declare_parameter<double>("track_width_mm", 14.0),
            this->declare_parameter<double>("wheel_diameter_mm", 55.0),
            this->declare_parameter<double>("encoder_ppr", 400.0)),
        serial(
            this->declare_parameter<std::string>("serial_port", "/dev/ttyUSB0"),
            this->declare_parameter<int>("baud_rate", 115200),
            sizeof(packet_mpu), this->get_logger()),
        timesync(this->get_logger()) {

    this->motor_sub =
        this->create_subscription<baller_interfaces::msg::MotorParameter>(
            "set_motor", 10,
            std::bind(&BallerSerial::handle_set_motor, this,
                      std::placeholders::_1));

    this->serial_io_port_reconnect_timer = this->create_wall_timer(
        3s, std::bind(&AsioSerialHub::connect, &this->serial));

    this->serial.attach<packet_time, BallerSerial>(TIME_PACKET_ID, this,
                                                   on_receive_time);

    odom_pub_ = this->create_publisher<nav_msgs::msg::Odometry>("odom", 10);
    imu_pub_ = this->create_publisher<sensor_msgs::msg::Imu>("imu/data", 10);
    this->robotLocalizationBridge.set_publisher(odom_pub_, imu_pub_);
  }

  ~BallerSerial() {}

private:
  RobotLocalizationBridge robotLocalizationBridge;
  AsioSerialHub serial;
  Timesync timesync;
  rclcpp::TimerBase::SharedPtr serial_io_port_reconnect_timer;

  rclcpp::Subscription<baller_interfaces::msg::MotorParameter>::SharedPtr
      motor_sub;

  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
  rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;

  void handle_set_motor(const baller_interfaces::msg::MotorParameter &motor) {
    packet_motor cmd = {};
    cmd.front_right = motor.front_right;
    cmd.front_left = motor.front_left;
    cmd.back_right = motor.back_right;
    cmd.back_left = motor.back_left;

    this->serial.write<packet_motor>(MOTOR_PACKET_ID, &cmd);
  }

  static void on_receive_mpu(BallerSerial *_this, packet_mpu *packet, fsize_t) {
    packet->timestamp = _this->timesync.sync_esp32_time(packet->timestamp);
    _this->robotLocalizationBridge.process_mpu_packet(packet);
  };

  static void on_receive_encoder(BallerSerial *_this, packet_encoder *packet,
                                 fsize_t) {
    packet->timestamp = _this->timesync.sync_stm32_time(packet->timestamp);
    _this->robotLocalizationBridge.process_encoder_packet(packet);
  }

  static void on_receive_time(BallerSerial *_this, packet_time *packet,
                              fsize_t) {

    uint64_t now = _this->get_clock()->now().nanoseconds() / 1000;

    if (_this->timesync.process_time_packet(packet, now)) {
      _this->robotLocalizationBridge.last_encoder = now;

      _this->serial.attach<packet_encoder, BallerSerial>(
          ENCODER_PACKET_ID, _this, on_receive_encoder);

      _this->serial.attach<packet_mpu, BallerSerial>(MPU_PACKET_ID, _this,
                                                     on_receive_mpu);
    }
  }
};

int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<BallerSerial>());
  rclcpp::shutdown();
  return 0;
}
