#include <cart/load.h>
#include <cart/mapper.h>
#include <cpu.h>
#include <dma.h>
#include <ee.h>
#include <render.h>
#include <palette.h>
#include <ppu.h>
#include <test.h>
#include <unistd.h>
#include <utils/logger.h>

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
    std::string rom;

    Options(int argc, char *argv[]) {
        int opt;

        while ((opt = getopt(argc, argv, "cdtpr:")) != -1) {
            switch (opt) {
            case 'c': log_steps_to_cerr = true; break;
            case 'd': step_debug = true; break;
            case 't': run_nestest = true; break;
            case 'p': run_ppu_tests = true; break;
            case 'r': rom = optarg; break;
            case '?':
            default:
                std::cerr << "Usage: " << argv[0] << " [-cdtp] [-r filename.nes]" << std::endl;
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

GFX::Renderer renderer(NES::ntsc_fb_x, NES::ntsc_fb_y);
NES::Palette pal("DigitalPrimeFBX.pal");
NES::OAMDMA oamdma;
NES::PPU ppu(renderer, pal);
NES::MemoryBus bus(ppu, oamdma);
NES::CPU cpu(bus, ppu);
NES::SystemLogger logger(cpu, ppu, bus);
ExecutionEnvironment ee(renderer, bus, cpu, ppu, oamdma, logger);

int main(int argc, char *argv[]) {
    Options opts(argc, argv);
    ee.debug = opts.step_debug;
    if (opts.log_steps_to_cerr) logger.instr_ostream = std::cerr;

    renderer.setup_window();

    if (!opts.rom.empty()) {
        std::cout << "Running " << opts.rom << std::endl;
        ee.logger.log_filename = NES::Test::gen_logname(opts.rom);
        ee.load_iNESv1(opts.rom);
        ee.power(nullptr);
        ee.pre_step_hook = [](auto &ee) {
            ee.logger.log();
        };
        ee.run();
        ee.logger.save();
    } else if (opts.run_nestest) {
        NES::Test::nestest(ee);
    } else if (opts.run_ppu_tests) {
        NES::Test::ppu_tests(ee);
    }

    return 0;
}
