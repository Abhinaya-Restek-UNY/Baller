#pragma once
#include "packet_types.h"

#define TIME_WARM_UP_PACKET_TOTAL 5
#include "rclcpp/rclcpp.hpp"
class Timesync {
public:
  Timesync(rclcpp::Logger logger)
      : logger(logger) {

        };
  int8_t process_time_packet(packet_time *packet, uint64_t now) {
    if (packet->unit_index == 1) {
      update_time_sync(this->esp32, packet, now);

      if (this->esp32.total_packet < TIME_WARM_UP_PACKET_TOTAL) {

        this->esp32.total_packet++;

        RCLCPP_INFO(this->logger, "[%d/%d] ESP32 time sync warm up...",
                    this->esp32.total_packet, TIME_WARM_UP_PACKET_TOTAL);
      }
    }

    if (packet->unit_index == 2) {
      update_time_sync(this->stm32, packet, now);

      if (this->stm32.total_packet < TIME_WARM_UP_PACKET_TOTAL) {

        RCLCPP_INFO(this->logger, "(%d) SYNC TIME delta(%lu) timestamp(%lu)",
                    packet->unit_index, packet->delta, packet->timestamp);
        this->stm32.total_packet++;
        RCLCPP_INFO(this->logger, "[%d/%d] STM32 time sync warm up...",
                    this->stm32.total_packet, TIME_WARM_UP_PACKET_TOTAL);
      }
    }

    if (this->esp32.total_packet + this->stm32.total_packet ==
        TIME_WARM_UP_PACKET_TOTAL * 2) {

      RCLCPP_INFO(this->logger, "Booting up packet handler!");

      this->esp32.total_packet++;
      this->stm32.total_packet++;
      return 1;
    }

    return 0;
  };

  void reset() {
    this->esp32.total_packet = 0;
    this->stm32.total_packet = 0;
  };

  uint64_t sync_stm32_time(uint64_t measured) {
    return this->sync_time(this->stm32, measured);
  };

  uint64_t sync_esp32_time(uint64_t measured) {
    return this->sync_time(this->esp32, measured);
  };

private:
  typedef struct {
    double d_offset;
    double d_drift;
    uint64_t last_tick;
    uint8_t state;
    uint8_t total_packet;
  } time_sync_data;
  rclcpp::Logger logger;

  time_sync_data stm32;
  time_sync_data esp32;

  static inline double lpf(double prev, double current, double alpha) {
    return (1 - alpha) * prev + alpha * current;
  };

  static inline void update_time_sync(time_sync_data &data, packet_time *packet,
                                      uint64_t current) {
    if (data.state == 0) {
      data.d_offset = current - packet->timestamp;
      data.last_tick = current;
      data.state = 1;
      return;
    }
    double delta = current - data.last_tick;
    if (data.state == 1) {
      data.d_drift = delta / packet->delta;
      data.state = 2;
    } else {
      data.d_drift = lpf(data.d_drift, delta / packet->delta, 0.1);
    }

    data.d_offset =
        lpf(data.d_offset, (current - packet->timestamp * data.d_drift), 0.1);
    data.last_tick = current;
  };

  static uint64_t sync_time(time_sync_data &data, uint64_t measured) {
    return (uint64_t)std::round(data.d_offset +
                                static_cast<double>(measured) * data.d_drift);
  };
};
