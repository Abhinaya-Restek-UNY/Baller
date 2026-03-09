add_library(motor STATIC motor.c)

target_include_directories(motor PUBLIC .)

target_include_directories(motor PUBLIC ${CMAKE_SOURCE_DIR}/Core/Inc)

target_link_libraries(motor PUBLIC stm32cubemx)
