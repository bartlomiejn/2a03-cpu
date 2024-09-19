cmake_minimum_required(VERSION 3.12)
project(2a03)

set(CMAKE_CXX_STANDARD 20)

# Project options
option(2A03_INCLUDE_TESTS "Include tests in the output." FALSE)

# Disable optimizations for GCC debug builds
if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Werror -Wall")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -Werror -Wall")
endif(CMAKE_COMPILER_IS_GNUCC)

set(NES_TEST_DIR external/nestest)
set(TEST_ROMS_DIR external/nes-test-roms)
set(PPU_TESTS_DIR ${TEST_ROMS_DIR}/blargg_ppu_tests_2005.09.15b)

set(SOURCES
        src/main.cpp
        src/cpu.cpp
        src/ppu.cpp
        src/bus.cpp
        src/cart/load.cpp
        src/cart/mapper.cpp
        src/utils/logger.cpp)

include_directories(include)

# Copy test ROMs to output dir
configure_file(${NES_TEST_DIR}/nestest.log nestest.log COPYONLY)
configure_file(${NES_TEST_DIR}/nestest.nes nestest.nes COPYONLY)
configure_file(${PPU_TESTS_DIR}/palette_ram.nes palette_ram.nes COPYONLY)

add_executable(2a03 ${SOURCES})
