cmake_minimum_required(VERSION 3.12)
project(2a03)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib/cppfs)

set(SOURCES src/main.cpp src/cpu.cpp src/bus.cpp src/cart/loader.cpp)

include_directories(include)

add_executable(2a03 ${SOURCES})
add_dependencies(2a03 cppfs)