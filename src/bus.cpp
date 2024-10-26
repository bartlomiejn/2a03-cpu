#include <apu.h>
#include <bus.h>
#include <cpu.h>
#include <ppu.h>

#include <cstring>
#include <iostream>

using namespace NES;

MemoryBus::MemoryBus(NES::PPU &_ppu, NES::APU &_apu)
    : ppu(_ppu), apu(_apu), cpu(nullptr) {
    // Ram state is not consistent on a real machine
    std::fill(ram.begin(), ram.end(), 0x0);
}

uint8_t MemoryBus::read(uint16_t addr) {
    switch (addr) {
    // Internal RAM
    case 0x0000 ... 0x1FFF:
        // 0x0000 - 0x00FF is zero page
        // 0x0100 - 0x01FF is stack memory
        // 0x0200 - 0x07FF is RAM
        return ram[addr % 0x800];

    // PPU registers
    case 0x2000 ... 0x3FFF: return ppu.cpu_read(addr);

    // APU registers
    case 0x4000 ... 0x4017: return apu.read(addr);

    // CPU test mode APU/IO functionality (disabled)
    case 0x4018 ... 0x401F:
        std::cerr << "CPU test mode memory access at $" << std::hex << (int)addr
                  << "." << std::endl;
        throw std::range_error("Unhandled CPU test mode read");

    // Cartridge space
    case 0x4020 ... 0xFFFF:
        if (mapper)
            return mapper->read_prg(addr);
        else
            throw MissingCartridge();

    default:
        std::cerr << "Unhandled memory access: $" << std::hex << (int)addr
                  << std::endl;
        throw std::range_error("Unhandled memory access");
    }
}

void MemoryBus::write(uint16_t addr, uint8_t val) {
    switch (addr) {
    case 0x0000 ... 0x1FFF: ram[addr % 0x800] = val; break;
    case 0x2000 ... 0x3FFF:
        ppu.cpu_write(0x2000 + ((addr - 0x2000) % 8), val);
        break;
    case 0x4000 ... 0x4017:
        if (addr == 0x4014)
            return cpu->schedule_dma_oam(val);
        else
            return apu.write(val);
    case 0x4020 ... 0xFFFF:
        if (mapper)
            mapper->write_prg(addr, val);
        else
            throw MissingCartridge();
        break;
    default:
        std::cerr << "Unhandled write to $" << std::hex << (int)addr
                  << " with value: " << std::hex << (int)val << "."
                  << std::endl;
        throw std::range_error("Unhandled write");
    }
}
