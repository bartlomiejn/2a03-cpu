#include <apu.h>
#include <cpu.h>
#include <ee.h>
#include <load.h>
#include <log.h>
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
    bool log_cpu = false;
    bool log_ppu = false;
    bool log_bus = false;
    bool log_nrom = false;
    bool log_cpu_state = false;  // SystemLogGenerator CPU state logging
    bool log_ppu_state = false;  // SystemLogGenerator PPU state logging
    bool step_debug = false;
    bool run_nestest = false;
    bool run_nestest_i = false;
    bool run_ppu_tests = false;
    bool run_cpu_tests = false;
    uint64_t headless_frames = 0;  // Headless profiling mode (0 = disabled)
    std::string rom;
    std::string logfile;

    Options(int argc, char *argv[]) {
        int opt;

        while ((opt = getopt(argc, argv, "cepbmsdtiuyr:l:h:")) != -1) {
            switch (opt) {
            case 'c': log_cpu = true; break;
            case 'e': log_ppu = true; break;
            case 'p': log_ppu_state = true; break;
            case 'b': log_bus = true; break;
            case 'm': log_nrom = true; break;
            case 's': log_cpu_state = true; break;
            case 'd': step_debug = true; break;
            case 't': run_nestest = true; break;
            case 'i': run_nestest_i = true; break;
            case 'u': run_cpu_tests = true; break;
            case 'y': run_ppu_tests = true; break;
            case 'r': rom = optarg; break;
            case 'l': logfile = optarg; break;
            case 'h': headless_frames = std::stoull(optarg); break;
            case '?':
            default:
                std::cerr << "Usage: " << argv[0]
                          << " [-cepbmsdtiuy] [-r filename.nes] [-l logfile] "
                             "[-h frames]"
                          << std::endl;
                std::cerr << "Where:" << std::endl;
                std::cerr << "-c - Enable CPU debug logging" << std::endl;
                std::cerr << "-e - Enable PPU debug logging" << std::endl;
                std::cerr << "-p - Enable PPU state logging to cerr "
                             "(SystemLogGenerator)"
                          << std::endl;
                std::cerr << "-b - Enable Bus debug logging" << std::endl;
                std::cerr << "-m - Enable NROM mapper debug logging" << std::endl;
                std::cerr << "-s - Enable CPU state logging to cerr "
                             "(SystemLogGenerator)"
                          << std::endl;
                std::cerr << "-d - Step through each instruction" << std::endl;
                std::cerr << "-t - Run nestest" << std::endl;
                std::cerr << "-i - Set interactive mode for nestest"
                          << std::endl;
                std::cerr << "-u - Run CPU tests" << std::endl;
                std::cerr << "-y - Run PPU tests" << std::endl;
                std::cerr << "-r - Load a ROM from filename" << std::endl;
                std::cerr << "-l - Log to file" << std::endl;
                std::cerr << "-h - Headless profiling mode (run N frames "
                             "without GUI)"
                          << std::endl;
                throw std::runtime_error("Invalid usage");
            }
        }
    }
};

int main(int argc, char *argv[]) {
    Options opts(argc, argv);

    std::ofstream *logfile;
    if (opts.log_cpu) {
        NES::Log::instance().enable("CPU");
        NES::Log::instance().enable("cputest");
    }
    if (opts.log_ppu) {
        NES::Log::instance().enable("PPU");
        NES::Log::instance().enable("pputest");
    }
    if (opts.log_bus) NES::Log::instance().enable("Bus");
    if (opts.log_nrom) NES::Log::instance().enable("NROM");

    if (!opts.logfile.empty()) {
        logfile = new std::ofstream(opts.logfile);
        if (!logfile) {
            throw std::runtime_error("Could not open file for logging");
        }
        NES::Log::instance().set_output(logfile);
    }

    GFX::GUI gui(NES::ntsc_fb_x, NES::ntsc_fb_y);
    NES::Palette pal("DigitalPrimeFBX.pal");
    NES::PPU ppu(gui, pal);
    NES::APU apu;
    NES::Controller controller1, controller2;
    NES::MemoryBusIntf *bus;
    NES::Test::MemoryBus *mock_bus = nullptr;
    if (opts.run_cpu_tests) {
        mock_bus = new NES::Test::MemoryBus();
        bus = mock_bus;
    } else {
        bus = new NES::MemoryBus(ppu, apu, controller1, controller2);
    }
    NES::CPU cpu(bus);
    if (!mock_bus) {
        ((NES::MemoryBus*)bus)->cpu = &cpu;
        gui.controller1 = &controller1;
    }
    NES::SystemLogGenerator logger(cpu, ppu, bus);
    NES::ExecutionEnvironment ee(gui, bus, cpu, ppu, logger);

    ee.debug = opts.step_debug;

    // SystemLogGenerator state logging (for nestest)
    if (opts.log_cpu_state) logger.instr_ostream = std::cerr;
    if (opts.log_ppu_state) logger.ppu_ostream = std::cerr;

    // Skip GUI setup in headless mode
    if (opts.headless_frames == 0)
        gui.setup();

    if (!opts.rom.empty()) {
        std::cout << "Running " << opts.rom << std::endl;
        ee.load_iNESv1(opts.rom);
        ee.power(nullptr);
        ee.pre_step_hook = [&](auto &ee) {
            if (opts.log_cpu) {
                NES_LOG("CPU") << ee.logger.log() << std::endl;
            }
        };
        if (opts.headless_frames > 0) {
            ee.run_headless(opts.headless_frames);
        } else {
            ee.run();
        }
    } else if (opts.run_nestest) {
        NES::Test::nestest(ee, opts.run_nestest_i);
    } else if (opts.run_ppu_tests) {
        NES::Test::ppu(ee);
    } else if (opts.run_cpu_tests) {
        NES::Test::cpu(ee, mock_bus);
    }

    // if (bus)
    //     delete bus;
    // if (mock_bus)
    //     delete mock_bus;

    return 0;
}
