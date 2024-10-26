cmake_minimum_required(VERSION 3.12)
project(2a03)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL2 REQUIRED)
if (SDL2_FOUND)
    message(STATUS "Found SDL2")
    message(STATUS "Libraries: ${SDL2_LIBRARIES}") 
    message(STATUS "Include dirs: ${SDL2_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "SDL2 not found")
endif()

find_program(CPPCHECK_EXECUTABLE NAMES cppcheck)
if(CPPCHECK_EXECUTABLE)
    message(STATUS "Found cppcheck")
endif()

# Disable optimizations for GCC debug builds
if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Werror -Wall -fsanitize=address")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -Werror -Wall")
endif(CMAKE_COMPILER_IS_GNUCC)

set(EXTERNAL_DIR external)
set(NES_TEST_DIR external/nestest)
set(CPU_TEST_DIR external/nes6502) # Github: SingleStepTests/65x02, MIT License
set(TEST_ROMS_DIR external/nes-test-roms)
set(PPU_TESTS_DIR ${TEST_ROMS_DIR}/blargg_ppu_tests_2005.09.15b)
set(PPU_RDBUF_TESTS_DIR ${TEST_ROMS_DIR}/ppu_read_buffer)

set(SOURCES
        src/main.cpp
        src/cpu.cpp
        src/ppu.cpp
        src/bus.cpp
        src/load.cpp
        src/mapper.cpp
        src/logger.cpp)

include_directories(src)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(external/nlohmann_json/single_include/nlohmann)

# Copy test ROMs and data to output dir
configure_file(${NES_TEST_DIR}/nestest.log nestest.log COPYONLY)
configure_file(${NES_TEST_DIR}/nestest.nes nestest.nes COPYONLY)
configure_file(${PPU_TESTS_DIR}/palette_ram.nes palette_ram.nes COPYONLY)
configure_file(${PPU_RDBUF_TESTS_DIR}/test_ppu_read_buffer.nes test_ppu_read_buffer.nes COPYONLY)

# Copy palette file for PPU
configure_file(${EXTERNAL_DIR}/DigitalPrimeFBX.pal DigitalPrimeFBX.pal COPYONLY)

add_executable(2a03 ${SOURCES})

# CPU single instruction test specs
add_custom_command(
    TARGET 2a03 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/${CPU_TEST_DIR}"
        "${CMAKE_BINARY_DIR}/nes6502"
    COMMENT "Copying 65x02 nes6502 tests to output directory"
)

add_custom_target(
    cppcheck
    COMMAND 
        "${CPPCHECK_EXECUTABLE}"
        --enable=all
        --inconclusive
        --std=c++20
        --verbose
        --language=c++
        --project="${CMAKE_BINARY_DIR}/compile_commands.json"
        --suppress=missingIncludeSystem
        --check-level=exhaustive
        --checkers-report=cppcheckers
    COMMENT "Running cppcheck on source and header files"
)

target_link_libraries(2a03 ${SDL2_LIBRARIES})
