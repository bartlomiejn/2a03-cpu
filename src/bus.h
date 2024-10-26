#ifndef INC_2A03_BUS_H
#define INC_2A03_BUS_H

#include <apu.h>
#include <mapper.h>
#include <ppu.h>

#include <array>
#include <cstdint>
#include <functional>
#include <optional>

static const int internal_ram_sz = 0x800;  ///< NES Internal RAM size.

namespace NES {

class CPU;

class MemoryBus {
   public:
    NES::PPU &ppu;
    NES::APU &apu;
    NES::CPU *cpu;
    iNESv1::Mapper::Base *mapper;              ///< Cartridge mapper, if it's
                                               ///< inserted.
    std::array<uint8_t, internal_ram_sz> ram;  ///< Internal RAM

    /// Initializes the memory bus.
    explicit MemoryBus(NES::PPU &_ppu, NES::APU &_apu);

    /// Reads 8 bits of memory at the provided address.
    /// \param addr Address to read from.
    /// \return Byte that has been read.
    uint8_t read(uint16_t addr);

    /// Writes a value to the provided address.
    /// \param addr Address to write the value to.
    /// \param val Value to write.
    void write(uint16_t addr, uint8_t val);

    /// Schedule DMA on the CPU
    /// \param uint8_t Page to transfer
    std::function<void(uint8_t)> on_cpu_oamdma;

    /// Schedule NMI on the CPU, NMI will be handled at the end of the next
    /// instruction cycle
    std::function<void(void)> cpu_schedule_nmi;
};

class MissingCartridge {};
}  // namespace NES

#endif  // INC_2A03_BUS_H
