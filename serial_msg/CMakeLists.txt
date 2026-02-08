cmake_minimum_required(VERSION 3.16)

project(serial_msg LANGUAGES CXX)
add_library(serial_msg STATIC)

target_sources(serial_msg
	PRIVATE
	src/MessageManager.cpp
)

target_include_directories(serial_msg
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(serial_msg
	PUBLIC
	cxx_std_20
)
