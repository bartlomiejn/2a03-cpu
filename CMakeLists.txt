cmake_minimum_required(VERSION 3.12)
project(2a03)

set(EXTERNAL_DIR external)
set(NES_TEST_DIR external/nestest)
set(CPU_TEST_DIR external/nes6502) # Github: SingleStepTests/65x02, MIT License
set(TEST_ROMS_DIR external/nes-test-roms)
set(PPU_TESTS_DIR ${TEST_ROMS_DIR}/blargg_ppu_tests_2005.09.15b)
set(PPU_RDBUF_TESTS_DIR ${TEST_ROMS_DIR}/ppu_read_buffer)

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

#find_package(SDL2_ttf REQUIRED)
#if (SDL2_ttf_FOUND)
#    message(STATUS "Found SDL2_ttf, imported target SDL2_ttf::SDL2_ttf")
#else()
#    message(FATAL_ERROR "SDL2TTF not found")
#endif()

find_program(CPPCHECK_EXECUTABLE NAMES cppcheck)
if(CPPCHECK_EXECUTABLE)
    message(STATUS "Found cppcheck")
endif()

# Disable optimizations for GCC debug builds
if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Werror -Wall")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -Werror -Wall")
endif(CMAKE_COMPILER_IS_GNUCC)

set(IMGUI_DIR ${EXTERNAL_DIR}/imgui)
set(IMGUI_SOURCES 
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
        ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp)

set(SOURCES
        src/main.cpp
        src/cpu.cpp
        src/ppu.cpp
        src/bus.cpp
        src/load.cpp
        src/mapper.cpp
        src/logger.cpp
        src/gui.cpp)

include_directories(
        src 
        ${SDL2_INCLUDE_DIRS} 
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
        external/nlohmann_json/single_include/nlohmann)

#include_directories(${SDL2_INCLUDE_DIRS})
#include_directories(external/nlohmann_json/single_include/nlohmann)
#include_directories(${IMGUI_DIR}

# Copy various data to output dir
configure_file(${NES_TEST_DIR}/nestest.log nestest.log COPYONLY)
configure_file(${NES_TEST_DIR}/nestest.nes nestest.nes COPYONLY)
configure_file(${PPU_TESTS_DIR}/palette_ram.nes palette_ram.nes COPYONLY)
configure_file(${EXTERNAL_DIR}/color_test.nes color_test.nes COPYONLY)
configure_file(${PPU_RDBUF_TESTS_DIR}/test_ppu_read_buffer.nes test_ppu_read_buffer.nes COPYONLY)

# Copy GUI fonts to output dir
configure_file(${EXTERNAL_DIR}/Inter-VariableFont_opsz,wght.ttf Inter-VariableFont.ttf COPYONLY)
configure_file(${EXTERNAL_DIR}/Inter-Italic-VariableFont_opsz,wght.ttf Inter-Italic-VariableFont.ttf COPYONLY)
# Inter - designed by Rasmus Andersson rsms.me/inter
# Licensed under the SIL Open Font License 1.1 (https://openfontlicense.org)
# Free for personal and commercial use at the time of writing this

# Copy palette file for PPU
configure_file(${EXTERNAL_DIR}/DigitalPrimeFBX.pal DigitalPrimeFBX.pal COPYONLY)

add_executable(2a03 ${SOURCES} ${IMGUI_SOURCES})

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
