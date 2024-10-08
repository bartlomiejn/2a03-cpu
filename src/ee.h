#ifndef INC_2A03_EE_H
#define INC_2A03_EE_H

#include <bus.h>
#include <cart/load.h>
#include <cart/mapper.h>
#include <cpu.h>
#include <ppu.h>
#include <utils/logger.h>
#include <render.h>

const int ntsc_cyc_ratio = 3;

class ExecutionEnvironment {
   public:
    GFX::Renderer &renderer;
    NES::MemoryBus &bus;
    NES::CPU &cpu;
    NES::PPU &ppu;
    NES::SystemLogger &logger;
    std::optional<NES::iNESv1::Cartridge> cartridge;

    bool debug = false;
    bool stop = false;

    std::function<void(ExecutionEnvironment &)> pre_step_hook;
    std::function<void(ExecutionEnvironment &)> post_step_hook;

    ExecutionEnvironment() = delete;
    ExecutionEnvironment(GFX::Renderer &_renderer, NES::MemoryBus &_bus, 
                         NES::CPU &_cpu, NES::PPU &_ppu, 
                         NES::SystemLogger &_logger)
    : renderer(_renderer), bus(_bus), cpu(_cpu), ppu(_ppu), logger(_logger) {}

    void power(std::function<void(NES::CPU &, NES::PPU &)> setup_hook) {
        cpu.power();
        ppu.power();
        if (setup_hook) setup_hook(cpu, ppu);
    }

    void load_iNESv1(std::string rom) {
        cartridge = NES::iNESv1::load(rom);
        NES::iNESv1::Mapper::Base *mapper =
            NES::iNESv1::Mapper::mapper(cartridge.value());
        bus.mapper = mapper;
        ppu.mapper = mapper;
    }

    void run() {
        // NTSC
        // If rendering off, each frame is 341*262 / 3 CPU clocks long
        // Scanline (341 pixels) is 113+(2/3) CPU clocks long
        // HBlank (85 pixels) is 28+(1/3) CPU clocks long
        // Frame is 29780.5 CPU clocks long
        // 3 PPU dots per 1 CPU cycle
        // OAM DMA is 513 CPU cycles + 1 if starting on CPU get cycle
        // Cycle reference:
        // https://www.nesdev.org/wiki/Cycle_reference_chart

        while (!stop) {
            if (pre_step_hook) pre_step_hook(*this);

            uint8_t cpu_cycs = cpu.execute();
            // TODO: Not sure if this is correct, as NMIs might get generated
            // before a CPU full instruction cycle finishes?
            ppu.execute(ntsc_cyc_ratio * cpu_cycs);
            stop = renderer.poll_quit();

            if (post_step_hook) post_step_hook(*this);

            if (debug) {
                char in = 0x0;
                std::cerr << "Stopped, next CPU step (y/n)" << std::endl;
                while (in != 'y' && in != 'n') {
                    std::cin.get(in);
                }
                if (in == 'n') {
                    stop = true;
                }
            }
        }
    }
};

#endif
