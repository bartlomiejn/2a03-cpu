#include <2a03/cart/load.h>
#include <2a03/cart/mapper.h>
#include <2a03/cpu.h>
#include <2a03/ee.h>
#include <2a03/test.h>
#include <2a03/utils/logger.h>
#include <unistd.h>

#include <cassert>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <functional>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>

struct Options {
    bool log_steps_to_cerr = false;
    bool run_nestest = false;
    bool run_ppu_tests = false;

    Options(int argc, char *argv[]) {
        int opt;

        while ((opt = getopt(argc, argv, "ctp")) != -1) {
            switch (opt) {
                case 'c':
                    log_steps_to_cerr = true;
                    break;
                case 't':
                    run_nestest = true;
                    break;
                case 'p':
                    run_ppu_tests = true;
                    break;
                default:
                    std::cerr << "Usage: " << argv[0] << " [-ctp]" << std::endl;
                    std::cerr << "Where:" << std::endl;
                    std::cerr << "-c - Log CPU state for each step to stderr"
                              << std::endl;
                    std::cerr << "-t - Run nestest" << std::endl;
                    std::cerr << "-p - Run PPU tests" << std::endl;
                    throw std::runtime_error("Invalid usage");
            }
        }
    }
};

int main(int argc, char *argv[]) {
    NES::MemoryBus bus;
    NES::CPU cpu(bus);
    NES::CPULogger logger(cpu, bus);
    ExecutionEnvironment ee(bus, cpu, logger);

    Options opts(argc, argv);
    if (opts.log_steps_to_cerr) logger.instr_ostream = std::cerr;

    if (opts.run_nestest) NES::Test::nestest(ee);
    if (opts.run_ppu_tests) NES::Test::ppu_tests(ee);

    return 0;
}
