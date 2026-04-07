#pragma once
#include "nav_msgs/msg/odometry.hpp"
#include "packet_types.h"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include <cmath>

static double PI = std::acos(-1.0);
class RobotLocalizationBridge {
public:
  RobotLocalizationBridge(double track_width_mm, double wheel_diameter_mm,
                          double encoder_ppr) {

    this->track_width_mm = track_width_mm;
    this->encoder_resolution_dis =
        (PI * wheel_diameter_mm) /
        (encoder_ppr * 4 *
         1000); // *4 because were counting A and B idk man. this is m/tick

    odom_msg_.header.frame_id = "odom";
    odom_msg_.child_frame_id = "base_link";

    // Set the covariance matrix once (assuming 0.01 variance for x, y, yaw)
    odom_msg_.twist.covariance[0] = 0.001;  // X velocity
    odom_msg_.twist.covariance[7] = 0.001;  // Y velocity
    odom_msg_.twist.covariance[35] = 0.001; // Yaw velocity

    imu_msg_.header.frame_id = "imu_link"; // Must match your TF tree

    // 1. Orientation Covariance (Trust the DMP quaternion)
    imu_msg_.orientation_covariance[0] = 0.001; // Roll variance
    imu_msg_.orientation_covariance[4] = 0.001; // Pitch variance
    imu_msg_.orientation_covariance[8] = 0.001; // Yaw variance

    // 2. Linear Acceleration Covariance (Accelerometer noise)
    imu_msg_.linear_acceleration_covariance[0] = 0.1; // X accel variance
    imu_msg_.linear_acceleration_covariance[4] = 0.1; // Y accel variance
    imu_msg_.linear_acceleration_covariance[8] = 0.1; // Z accel variance

    // 3. Angular Velocity Covariance (Gyroscope)
    // CRITICAL: Setting the first element to -1.0 is the standard ROS way to
    // tell the EKF: "I am not providing this data, please ignore it."
    imu_msg_.angular_velocity_covariance[0] = -1.0;
  }

  void
  set_publisher(rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_,
                rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_) {
    this->odom_pub_ = odom_pub_;
    this->imu_pub_ = imu_pub_;
  }

  double encoder_resolution_dis;

  void process_mpu_packet(packet_mpu *packet) {

    this->imu_msg_.header.stamp.sec = packet->timestamp / 1000000;
    this->imu_msg_.header.stamp.nanosec = (packet->timestamp % 1000000) * 1000;

    this->imu_msg_.linear_acceleration.y = packet->a_y * this->accel_resolution;
    this->imu_msg_.linear_acceleration.x = packet->a_x * this->accel_resolution;
    this->imu_msg_.linear_acceleration.z = packet->a_z * this->accel_resolution;

    this->imu_msg_.orientation.x = packet->q_x;
    this->imu_msg_.orientation.y = packet->q_y;
    this->imu_msg_.orientation.z = packet->q_z;
    this->imu_msg_.orientation.w = packet->q_w;

    this->imu_pub_->publish(this->imu_msg_);
  };

  void process_encoder_packet(packet_encoder *packet) {
    double delta_us =
        packet->timestamp - this->last_encoder; // Delta in microseconds
    this->last_encoder = packet->timestamp;

    if (delta_us <= 0.0) {
      return;
    }
    double delta_sec = delta_us / 1000000.0;

    this->odom_msg_.header.stamp.sec = packet->timestamp / 1000000;
    this->odom_msg_.header.stamp.nanosec = (packet->timestamp % 1000000) * 1000;

    this->odom_msg_.twist.twist.angular.z =
        (((packet->deltaA - packet->deltaC) * this->encoder_resolution_dis) /
         this->track_width_mm) /
        delta_sec;

    this->odom_msg_.twist.twist.linear.y =
        (((double)packet->deltaB * this->encoder_resolution_dis) / delta_sec);

    this->odom_msg_.twist.twist.linear.x =
        (((packet->deltaA + packet->deltaC) * this->encoder_resolution_dis) /
         2.0) /
        delta_sec;

    this->odom_pub_->publish(this->odom_msg_);
  }

  double last_encoder = 0;

private:
  double track_width_mm = 0;
  double accel_resolution = 9.80665 / 16384.0;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
  rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;
  nav_msgs::msg::Odometry odom_msg_;
  sensor_msgs::msg::Imu imu_msg_;
};
