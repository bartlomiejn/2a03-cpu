cmake_minimum_required(VERSION 3.12)
project(2a03)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/)

set(SOURCES src/main.cpp src/cpu.cpp src/bus.cpp src/cart/loader.cpp)

include_directories(include)

add_executable(2a03 ${SOURCES})
