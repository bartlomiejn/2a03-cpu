cmake_minimum_required(VERSION 3.12)
project(2a03)

set(CMAKE_CXX_STANDARD 17)

set(CPPFS_DIR lib/cppfs)
set(CPPFS_INCLUDE ${CPPFS_DIR}/source/cppfs/include)
add_subdirectory(${CPPFS_DIR})

set(SOURCES src/main.cpp src/cpu.cpp src/bus.cpp src/cart/loader.cpp)

include_directories(include ${CPPFS_INCLUDE})

add_executable(2a03 ${SOURCES})
add_dependencies(2a03 cppfs)