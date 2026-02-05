#ifndef INC_2A03_EE_H
#define INC_2A03_EE_H

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <bus.h>

#ifdef ENABLE_CALLGRIND
#include <valgrind/callgrind.h>
#else
#define CALLGRIND_START_INSTRUMENTATION do {} while (0)
#define CALLGRIND_STOP_INSTRUMENTATION do {} while (0)
#endif
#include <cpu.h>
#include <load.h>
#include <logger.h>
#include <mapper.h>
#include <ppu.h>
#include <gui.h>

// NTSC
// If rendering off, each frame is 341*262 / 3 CPU clocks long
// Scanline (341 pixels) is 113+(2/3) CPU clocks long
// HBlank (85 pixels) is 28+(1/3) CPU clocks long
// Frame is 29780.5 CPU clocks long
// 3 PPU dots per 1 CPU cycle
// OAM DMA is 513 CPU cycles + 1 if starting on CPU get cycle
// Cycle reference:
// https://www.nesdev.org/wiki/Cycle_reference_chart
//
const int ntsc_cyc_ratio = 3;

namespace NES {

class ExecutionEnvironment {
   public:
    GFX::GUI &gui;
    NES::MemoryBusIntf *bus;
    NES::CPU &cpu;
    NES::PPU &ppu;
    NES::SystemLogGenerator &logger;
    std::optional<NES::iNESv1::Cartridge> cartridge;
    NES::iNESv1::Mapper::Base *mapper; 

    std::thread execThread;

    std::atomic<bool> debug = false;
    std::atomic<bool> stop = false;
    std::atomic<bool> disable_ppu = false;
    std::atomic<bool> run_single_step = false;

    std::function<void(ExecutionEnvironment &)> pre_step_hook;
    std::function<void(ExecutionEnvironment &)> post_step_hook;

    ExecutionEnvironment() = delete;
    ExecutionEnvironment(GFX::GUI &_gui,
                         NES::MemoryBusIntf *_bus,
                         NES::CPU &_cpu, NES::PPU &_ppu,
                         NES::SystemLogGenerator &_logger)
        : gui(_gui),
          bus(_bus),
          cpu(_cpu),
          ppu(_ppu),
          logger(_logger) {
        gui.ppu = &ppu;
        gui.cpu = &cpu;
    }

    ~ExecutionEnvironment() {
        if (cartridge)
            cartridge.reset();
    }

    void power(std::function<void(NES::CPU &, NES::PPU &)> setup_hook) { 
        // PPU /VBL line is connected directly to /NMI
        if (!disable_ppu)
            ppu.on_nmi_vblank = [&]() { cpu.schedule_nmi(); };

        cpu.power();
        if (!disable_ppu)
            ppu.power();

        if (setup_hook) setup_hook(cpu, ppu);
    }

    void load_iNESv1(std::string rom) {
        cartridge = NES::iNESv1::load(rom);
        mapper = NES::iNESv1::Mapper::mapper(cartridge.value());
        bus->mapper = mapper;
        ppu.mapper = mapper;
        gui.mapper = mapper;
        gui.ppu = &ppu;
        gui.cpu = &cpu;
    }

    void run() {
        stop = false;
        gui.stop = false;
        execThread = std::thread(&ExecutionEnvironment::runloop, this);
        gui.enter_runloop();
        stop = true;
        execThread.join();
    }

    /// Run emulation headless for profiling (no GUI)
    void run_headless(uint64_t frames) {
        ppu.headless = true;
        ppu.frame_count = 0;
        stop = false;

        CALLGRIND_START_INSTRUMENTATION;

        while (!stop) {
            if (pre_step_hook) pre_step_hook(*this);

            try {
                uint16_t cpu_cycs = cpu.execute();
                if (!disable_ppu)
                    ppu.execute(ntsc_cyc_ratio * cpu_cycs);
            } catch (NES::InvalidOpcode &e) {
                std::cerr << "Unhandled opcode executed." << std::endl;
                break;
            } catch (NES::JAM &e) {
                break;
            }

            if (post_step_hook) post_step_hook(*this);

            if (frames > 0 && ppu.frame_count >= frames) {
                stop = true;
            }
        }

        CALLGRIND_STOP_INSTRUMENTATION;
    }

private:
    void runloop() {
        while (!stop) {
            if (pre_step_hook) pre_step_hook(*this);

            try {
                uint16_t cpu_cycs = cpu.execute();
                if (!disable_ppu)
                    ppu.execute(ntsc_cyc_ratio * cpu_cycs);
            } catch (NES::InvalidOpcode &e) {
                std::cerr << "Unhandled opcode executed." << std::endl;
            } catch (NES::JAM &e) {
                // TODO: Actually jam and handle reset
            }

            if (post_step_hook) post_step_hook(*this);

            if (run_single_step) {
                stop = true;
            }

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
        gui.stop = true;
    }
};

} // namespace NES

#endif
