cmake_minimum_required(VERSION 3.12)
project(2a03)

set(CMAKE_CXX_STANDARD 20)

find_package(SDL2 REQUIRED)
if (SDL2_FOUND)
    message(STATUS "Found SDL2")
    message(STATUS "Libraries: ${SDL2_LIBRARIES}") 
    message(STATUS "Include dirs: ${SDL2_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "SDL2 not found")
endif()

# Project options
option(2A03_INCLUDE_TESTS "Include tests in the output." FALSE)

# Disable optimizations for GCC debug builds
if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Werror -Wall")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -Werror -Wall")
endif(CMAKE_COMPILER_IS_GNUCC)

set(EXTERNAL_DIR external)
set(NES_TEST_DIR external/nestest)
set(TEST_ROMS_DIR external/nes-test-roms)
set(PPU_TESTS_DIR ${TEST_ROMS_DIR}/blargg_ppu_tests_2005.09.15b)
set(PPU_RDBUF_TESTS_DIR ${TEST_ROMS_DIR}/ppu_read_buffer)

set(SOURCES
        src/main.cpp
        src/cpu.cpp
        src/ppu.cpp
        src/bus.cpp
        src/cart/load.cpp
        src/cart/mapper.cpp
        src/utils/logger.cpp)

include_directories(src)
include_directories(${SDL2_INCLUDE_DIRS})

# Copy test ROMs to output dir
configure_file(${NES_TEST_DIR}/nestest.log nestest.log COPYONLY)
configure_file(${NES_TEST_DIR}/nestest.nes nestest.nes COPYONLY)
configure_file(${PPU_TESTS_DIR}/palette_ram.nes palette_ram.nes COPYONLY)
configure_file(${PPU_RDBUF_TESTS_DIR}/test_ppu_read_buffer.nes test_ppu_read_buffer.nes COPYONLY)

# Copy palette file for PPU
configure_file(${EXTERNAL_DIR}/DigitalPrimeFBX.pal DigitalPrimeFBX.pal COPYONLY)

add_executable(2a03 ${SOURCES})

target_link_libraries(2a03 ${SDL2_LIBRARIES})
