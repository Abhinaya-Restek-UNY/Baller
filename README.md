# Todo List
## STM32
- [x] wheel.h and wheel.c responsible control invidual wheel
- [x] encoder.h and encoder.c responsible for querying individual encoder and send the data to ESP32 via UART (done via serial_hub and inside main without encoder.h and encoder.c)
- [x] forward and interpret data from serial (via serial_hub)

## ESP32
- [ ] MPU6050.hpp and MPU6050.cpp responsible for calculating current angle by reading MPU6050 data
- [x] DataCenter.hpp and DataCenter.cpp responsible for sending all the data to ros2 via serial (via serial_hub)

## ROS2
- [ ] DataCenter responsible for receiving and populating data from ESP32 Serial
- [ ] Commander responsible for sending commads to STM32
- [ ] Joystick responsible for getting joystick input and then send them to Commander

# Data Transfer Specifications
## serial_hub packet structure
|Data type|Description|
|------------|---|
| 0x0 | beginning of packet |
| uint8_t | topic id |
| uint16_t | expected length|
## encoder data via serial_hub (STM32->ESP32->ROS2)
|Data type|Description|
|------------|---|
| uint64_t | timestamp |
| int32_t | total encoder A ticks |
| int32_t | total encoder B ticks |
| int32_t | total encoder C ticks |
## orientation(quaternion) and acceleration data via serial hub (MPU6050->ESP32->ROS2)
|Data type|Description|
|------------|---|
| uint64_t | timestamp |
| float | w |
| float | x |
| float | y |
| float | z |
| int16_t | accel x |
| int16_t | accel y |
| int16_t | z |
## motor direction data (ROS2->ESP32->STM32)
|Data type|Description|
|------------|---|
| int16_t | motorA |
| int16_t | motorB |
| int16_t | motorC |
| int16_t | motorD |
