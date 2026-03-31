add_library(app_core STATIC 
    core.cpp
)

target_include_directories(app_core PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
)

target_include_directories(app_core PRIVATE 
    ${CMAKE_SOURCE_DIR}/Core/Inc
)


add_subdirectory(../lib/motor ${CMAKE_SOURCE_DIR}/build/motor)
add_subdirectory(../lib/serial_hub ${CMAKE_SOURCE_DIR}/build/serial_hub)

target_link_libraries(app_core PUBLIC stm32cubemx)
target_link_libraries(app_core PUBLIC motor)
target_link_libraries(app_core PUBLIC serial_hub)
