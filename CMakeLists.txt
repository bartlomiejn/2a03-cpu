cmake_minimum_required(VERSION 3.12)
project(2a03)

set(CMAKE_CXX_STANDARD 17)

# Project options
option(2A03_INCLUDE_TESTS "Include tests in the output." FALSE)

# Disable optimizations for GCC debug builds
if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
endif(CMAKE_COMPILER_IS_GNUCC)

set(SOURCES
        src/main.cpp
        src/cpu.cpp
        src/ppu.cpp
        src/bus.cpp
        src/cart/load.cpp
        src/cart/mapper.cpp
        src/utils/logger.cpp)

include_directories(include)

add_executable(2a03 ${SOURCES})