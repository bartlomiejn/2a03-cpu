#include <apu.h>
#include <bus.h>
#include <cpu.h>
#include <log.h>
#include <ppu.h>

#include <cstring>
#include <iostream>

using namespace NES;

MemoryBus::MemoryBus(NES::PPU &_ppu, NES::APU &_apu,
                     NES::Controller &_ctrl1, NES::Controller &_ctrl2)
    : ppu(_ppu), apu(_apu), cpu(nullptr),
      controller1(_ctrl1), controller2(_ctrl2) {
    // Ram state is not consistent on a real machine
    std::fill(ram.begin(), ram.end(), 0x0);
}

uint8_t MemoryBus::read(uint16_t addr, bool passive) {
    switch (addr) {
    // Internal RAM
    case 0x0000 ... 0x1FFF:
        // 0x0000 - 0x00FF is zero page
        // 0x0100 - 0x01FF is stack memory
        // 0x0200 - 0x07FF is RAM
        NES_LOG("Bus") << "Read internal RAM @ 0x" << std::hex << std::setw(4)
                       << std::setfill('0') << addr % 0x800 << ", value: 0x"
                       << std::setw(2) << std::setfill('0')
                       << (uint16_t)ram[addr % 0x800] << std::endl;
        return ram[addr % 0x800];

    // PPU registers
    case 0x2000 ... 0x3FFF: return ppu.cpu_read(0x2000 + ((addr - 0x2000) % 8), passive);

    // APU registers
    case 0x4000 ... 0x4015:
        if (addr == 0x4014) {  // OAMDMA
            NES_LOG("Bus") << "Read open bus, dummy value 0xff\n";
            return 0xff;  // TODO: Implement open bus behavior
        }
        return apu.read(addr);

    // Controller registers
    case 0x4016: return controller1.read(passive);
    case 0x4017: return controller2.read(passive);

    // CPU test mode APU/IO functionality (disabled)
    case 0x4018 ... 0x401F:
        NES_LOG("Bus") << "CPU test mode memory access at $" << std::hex << (int)addr
                  << "." << std::endl;
        throw std::range_error("Unhandled CPU test mode read");

    // Cartridge space
    case 0x4020 ... 0xFFFF:
        if (mapper)
            return mapper->read_prg(addr);
        else
            throw MissingCartridge();

    default:
        NES_LOG("Bus") << "Unhandled memory access: $" << std::hex << (int)addr
                  << std::endl;
        throw std::range_error("Unhandled memory access");
    }
}

void MemoryBus::write(uint16_t addr, uint8_t val) {
    switch (addr) {
    case 0x0000 ... 0x1FFF:
        NES_LOG("Bus") << "Write to internal RAM @ 0x" << std::hex
                       << std::setw(4) << std::setfill('0') << addr % 0x800
                       << " value: 0x" << std::setw(2) << std::setfill('0')
                       << (uint16_t)val << std::endl;
        ram[addr % 0x800] = val;
        break;
    case 0x2000 ... 0x3FFF:
        ppu.cpu_write(0x2000 + ((addr - 0x2000) % 8), val);
        break;
    case 0x4000 ... 0x4017:
        if (addr == 0x4014)
            return cpu->schedule_dma_oam(val);
        else if (addr == 0x4016) {
            controller1.write(val);
            controller2.write(val);
            return;
        } else
            return apu.write(addr, val);
    case 0x4020 ... 0xFFFF:
        if (mapper)
            mapper->write_prg(addr, val);
        else
            throw MissingCartridge();
        break;
    default:
        NES_LOG("Bus") << "Unhandled write to $" << std::hex << (int)addr
                  << " with value: " << std::hex << (int)val << "."
                  << std::endl;
        throw std::range_error("Unhandled write");
    }
}
