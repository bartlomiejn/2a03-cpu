#include <apu.h>
#include <cpu.h>
#include <ee.h>
#include <load.h>
#include <logger.h>
#include <mapper.h>
#include <palette.h>
#include <ppu.h>
#include <gui.h>
#include <test/test.h>
#include <test/test_cpu.h>
#include <test/test_ppu.h>
#include <test/test_nestest.h>

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
    bool step_debug = false;
    bool run_nestest = false;
    bool run_ppu_tests = false;
    bool run_cpu_tests = false;
    std::string rom;

    Options(int argc, char *argv[]) {
        int opt;

        while ((opt = getopt(argc, argv, "cdtpur:")) != -1) {
            switch (opt) {
            case 'c': log_steps_to_cerr = true; break;
            case 'd': step_debug = true; break;
            case 't': run_nestest = true; break;
            case 'p': run_ppu_tests = true; break;
            case 'u': run_cpu_tests = true; break;
            case 'r': rom = optarg; break;
            case '?':
            default:
                std::cerr << "Usage: " << argv[0]
                          << " [-cdtp] [-r filename.nes]" << std::endl;
                std::cerr << "Where:" << std::endl;
                std::cerr << "-c - Log CPU state for each step to stderr"
                          << std::endl;
                std::cerr << "-d - Step through each instruction" << std::endl;
                std::cerr << "-t - Run nestest" << std::endl;
                std::cerr << "-p - Run PPU tests" << std::endl;
                std::cerr << "-r - Load a ROM from filename" << std::endl;
                throw std::runtime_error("Invalid usage");
            }
        }
    }
};

int main(int argc, char *argv[]) {
    Options opts(argc, argv);

    GFX::GUI gui(NES::ntsc_fb_x, NES::ntsc_fb_y);
    NES::Palette pal("DigitalPrimeFBX.pal");
    NES::PPU ppu(gui, pal);
    NES::APU apu;
    NES::MemoryBusIntf *bus;
    NES::Test::MemoryBus *mock_bus;
    if (opts.run_cpu_tests) {
        mock_bus = new NES::Test::MemoryBus();
        bus = mock_bus;
    } else {
        bus = new NES::MemoryBus(ppu, apu);
    }
    NES::CPU cpu(bus);
    if (!mock_bus)
        ((NES::MemoryBus*)bus)->cpu = &cpu;
    NES::SystemLogGenerator logger(cpu, ppu, bus);
    NES::ExecutionEnvironment ee(gui, bus, cpu, ppu, logger);

    ee.debug = opts.step_debug;
    if (opts.log_steps_to_cerr) logger.instr_ostream = std::cerr;
    gui.setup();

    if (!opts.rom.empty()) {
        std::cout << "Running " << opts.rom << std::endl;
        ee.logger.log_filename = NES::Test::gen_logname(opts.rom);
        ee.load_iNESv1(opts.rom);
        ee.power(nullptr);
        ee.pre_step_hook = [](auto &ee) { ee.logger.log(); };
        ee.run();
        ee.logger.save();
    } else if (opts.run_nestest) {
        NES::Test::nestest(ee);
    } else if (opts.run_ppu_tests) {
        NES::Test::ppu(ee);
    } else if (opts.run_cpu_tests) {
        NES::Test::cpu(ee, mock_bus);
    }

    if (bus)
        delete bus;
    if (mock_bus)
        delete mock_bus;

    return 0;
}
