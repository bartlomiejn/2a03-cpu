#ifndef INC_2A03_EE_H
#define INC_2A03_EE_H

#include <bus.h>
#include <cart/load.h>
#include <cart/mapper.h>
#include <utils/logger.h>
#include <cpu.h>
#include <ppu.h>

class ExecutionEnvironment {
   public:
    NES::MemoryBus &bus;
    NES::CPU &cpu;
    NES::CPULogger &logger;

    std::function<void(void)> pre_step_hook;
    std::function<void(void)> post_step_hook;

    ExecutionEnvironment() = delete;
    ExecutionEnvironment(NES::MemoryBus &_bus, NES::CPU &_cpu,
                         NES::CPULogger &_logger)
        : bus(_bus), cpu(_cpu), logger(_logger) {}

    void power(std::function<void(NES::CPU &)> setup_hook) {
        cpu.power();
        setup_hook(cpu);
    }

    void load_iNESv1(std::string rom) {
        NES::iNESv1::Cartridge cartridge = NES::iNESv1::load(rom);
        NES::iNESv1::Mapper::Base *mapper =
            NES::iNESv1::Mapper::mapper(cartridge);
        bus.mapper = mapper;
    }

    void step() {
        pre_step_hook();
        cpu.execute();
        post_step_hook();
    }
};

#endif
