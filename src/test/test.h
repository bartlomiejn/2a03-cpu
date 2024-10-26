#ifndef INC_2A03_TEST_H
#define INC_2A03_TEST_H

#include <ee.h>
#include <unistd.h>
#include <test/bus.h>
#include <test/test_cpu.h>
#include <test/test_ppu.h>
#include <test/test_nestest.h>

#include <chrono>
#include <csignal>
#include <ctime>
#include <iostream>
#include <string>

namespace NES {

namespace Test {

std::string gen_logname(std::string prefix) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm lt = *std::localtime(&now_time);
    std::stringstream time_s;
    time_s << std::put_time(&lt, "%d_%H:%M:%S");
    return std::format("{}_{}.log", prefix, time_s.str());
}

};  // namespace Test

}  // namespace NES

#endif 
