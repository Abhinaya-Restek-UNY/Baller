# Todo List
## STM32
- [ ] wheel.h and wheel.c responsible control invidual wheel
- [ ] encoder.h and encoder.c responsible for querying individual encoder and send the data to ESP32 via UART
- [ ] movement.h and movement.c responsible for querying direction data via UART from ESP32. Also responsible for calculating the speed of individual wheel via wheel.h

## ESP32
- [ ] MPU6050.hpp and MPU6050.cpp responsible for calculating current angle by reading MPU6050 data
- [ ] Position.hpp and Position.cpp responsible for processing MPU6050.hpp acceleration value and then get relative position from the origin (origin is where the robot fist activated).
- [ ] Direction.hpp and Direction.cpp responsible for calculate relative direction to achieve absolute direction and then send the direction to STM32. Also responsible for receiving encoder data from STM32 via UART and then process them
- [ ] Commands.hpp and Commands.cpp responsible for interpreting commands from ros2 that is received via serial
- [ ] DataCenter.hpp and DataCenter.cpp responsible for sending all the data to ros2 via serial

## ROS2
- [ ] DataCenter responsible for receiving and populating data from ESP32 Serial
- [ ] Commander responsible for receiving commands and send them to ESP32 via serial
- [ ] Joystick responsible for getting joystick input and then send them to Commander

# Data Transfer Specifications
## encoder.h encoder data STM32 -> ESP32
|Data type|Description|
|------------|---|
| unsigned char  | Encoder index |
| float | angle |

## Direction.hpp to movement.h ESP32 -> STM32
| Data type | Description |
|-----|------|
| float | x direction (-1.f to 1.f) |
| float | y direction (-1.f to 1.f) |

## DataCenter.hpp to DataCenter ESP32 -> ROS2
| Data type | Description |
|----|---|
| float  |  roll|
| float | pitch|
| float | yaw|
| float | x relative position |
| float | y relative position |

## Commander to Commands.hpp
| Data type | Description |
|-----|------|
| unsigned char | command |
| struct | command data (depend on the command specified) |


# Command data
## TARGET (0x1)
| Data type | Description |
|--------|-------|
|float|x relative position from current position|
|float|y relative position from current position|

## Velocity (0x2)
| Data type | Description |
|----------|-------|
|float|x velocity(-1.f to 1.f)|
|float|y velocity(-1.f to 1.f)|
