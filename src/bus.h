#ifndef INC_2A03_BUS_H
#define INC_2A03_BUS_H

#include <cart/mapper.h>
#include <dma.h>
#include <ppu.h>

#include <array>
#include <cstdint>
#include <functional>
#include <optional>

static const int internal_ram_sz = 0x800;  ///< NES Internal RAM size.

namespace NES {
class MemoryBus {
   public:
    NES::PPU &ppu;
    NES::OAMDMA &oamdma;
    iNESv1::Mapper::Base *mapper;              ///< Cartridge mapper, if it's
                                               ///< inserted.
    std::array<uint8_t, internal_ram_sz> ram;  ///< Internal RAM

    /// Initializes the memory bus.
    MemoryBus(NES::PPU &_ppu, NES::OAMDMA &_oamdma);

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
    std::function<void(uint8_t)> cpu_schedule_dma_oam;

    /// Schedule NMI on the CPU, NMI will be handled at the end of the next
    /// instruction cycle
    std::function<void(void)> cpu_schedule_nmi;

   protected:
    /// Handler for OAMDMA transfer schedule
    void dma_oam_handler(uint8_t page);

    /// Handler for OAMDMA read
    void dma_oam_read_handler(uint16_t addr);

    /// Handler for OAMDMA write
    void dma_oam_write_handler(uint16_t addr);
};

class MissingCartridge {};
}  // namespace NES

#endif  // INC_2A03_BUS_H
