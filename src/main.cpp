#include <SDL2/SDL.h>
#include <cart/load.h>
#include <cart/mapper.h>
#include <cpu.h>
#include <dma.h>
#include <ee.h>
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

// NTSC dimensions
const int fb_ntsc_x = 256;
const int fb_ntsc_y = 240;
// Scale size by this value
int scaling = 3;

const int display_x = fb_ntsc_x;
const int display_y = fb_ntsc_y;

SDL_Window *w;
SDL_Renderer *r;
SDL_Texture *tex;

void setup_window() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init error: " << SDL_GetError() << std::endl;
        throw std::runtime_error("SDL_Init error");
    }

    w = SDL_CreateWindow("2A03", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         display_x, display_y, SDL_WINDOW_SHOWN);
    if (!w) {
        std::cerr << "SDL_CreateWindow error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        throw std::runtime_error("SDL_CreateWindow error");
    }

    r = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED);

    if (!r) {
        std::cerr << "SDL_CreateRenderer error: " << SDL_GetError()
                  << std::endl;
        SDL_DestroyWindow(w);
        SDL_Quit();
        throw std::runtime_error("SDL_CreateRenderer error");
    }

    tex = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888,
                            SDL_TEXTUREACCESS_STREAMING, display_x, display_y);
    if (!tex) {
        std::cerr << "SDL_CreateTexture error: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(r);
        SDL_DestroyWindow(w);
        SDL_Quit();
        throw std::runtime_error("SDL_CreateTexture error");
    }
}

NES::Palette pal("DigitalPrimeFBX.pal");
NES::OAMDMA oamdma;
NES::PPU ppu(pal);
NES::MemoryBus bus(ppu, oamdma);
NES::CPU cpu(bus, ppu);
NES::CPULogger logger(cpu, bus);
ExecutionEnvironment ee(bus, cpu, ppu, oamdma, logger);

int main(int argc, char *argv[]) {
    Options opts(argc, argv);
    ee.debug = opts.step_debug;
    if (opts.log_steps_to_cerr) logger.instr_ostream = std::cerr;

    setup_window();

    // TODO: Threading
    ppu.frame_ready = [&](uint32_t* fb) {
        SDL_UpdateTexture(tex, NULL, fb, fb_ntsc_x * sizeof(uint32_t));
        SDL_RenderClear(r);
        SDL_RenderCopy(r, tex, NULL, NULL);
        SDL_RenderPresent(r);
    };

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
