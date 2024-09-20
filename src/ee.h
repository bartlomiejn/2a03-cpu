#ifndef INC_2A03_EE_H
#define INC_2A03_EE_H

#include <bus.h>
#include <cart/load.h>
#include <cart/mapper.h>
#include <cpu.h>
#include <ppu.h>
#include <utils/logger.h>

const int ntsc_cyc_ratio = 3;

class ExecutionEnvironment {
   public:
    NES::MemoryBus &bus;
    NES::CPU &cpu;
    NES::PPU &ppu;
    NES::OAMDMA &oamdma;
    NES::CPULogger &logger;
    std::optional<NES::iNESv1::Cartridge> cartridge;

    bool debug = false;
    bool stop = false;

    std::function<void(ExecutionEnvironment &)> pre_step_hook;
    std::function<void(ExecutionEnvironment &)> post_step_hook;

    ExecutionEnvironment() = delete;
    ExecutionEnvironment(NES::MemoryBus &_bus, NES::CPU &_cpu, NES::PPU &_ppu,
                         NES::OAMDMA &_oamdma, NES::CPULogger &_logger)
        : bus(_bus), cpu(_cpu), ppu(_ppu), oamdma(_oamdma), logger(_logger) {}

    void power(std::function<void(NES::CPU &)> setup_hook) {
        cpu.power();
        if (setup_hook) setup_hook(cpu);
    }

    void load_iNESv1(std::string rom) {
        cartridge = NES::iNESv1::load(rom);
        NES::iNESv1::Mapper::Base *mapper =
            NES::iNESv1::Mapper::mapper(cartridge.value());
        bus.mapper = mapper;
        ppu.mapper = mapper;
    }

    void run() {
        while (!stop) {
            if (pre_step_hook) pre_step_hook(*this);

            uint8_t cpu_cycs = cpu.execute();
            ppu.execute(ntsc_cyc_ratio * cpu_cycs);

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
