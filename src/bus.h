#ifndef INC_2A03_BUS_H
#define INC_2A03_BUS_H

#include <apu.h>
#include <controller.h>
#include <mapper.h>
#include <ppu.h>

#include <array>
#include <cstdint>
#include <functional>
#include <optional>

namespace NES {

class CPU;

class MemoryBusIntf {
   public:
    iNESv1::Mapper::Base *mapper = nullptr;    

    virtual ~MemoryBusIntf() = default;

    /// Reads 8 bits of memory at the provided address.
    /// \param addr Address to read from.
    /// \param passive Don't trigger additional read behaviour
    /// \return Byte that has been read.
    virtual uint8_t read(uint16_t addr, bool passive=false) = 0;

    /// Writes a value to the provided address.
    /// \param addr Address to write the value to.
    /// \param val Value to write.
    virtual void write(uint16_t addr, uint8_t val) = 0;
};

class MemoryBus : public MemoryBusIntf {
   public:
    static const int ram_size = 0x800;  ///< NES Internal RAM size.

    NES::PPU &ppu;
    NES::APU &apu;
    NES::CPU *cpu;
    NES::Controller &controller1;
    NES::Controller &controller2;
    std::array<uint8_t, ram_size> ram;  ///< Internal RAM

    /// Initializes the memory bus.
    MemoryBus(NES::PPU &_ppu, NES::APU &_apu,
              NES::Controller &_ctrl1, NES::Controller &_ctrl2);

    /// Reads 8 bits of memory at the provided address.
    /// \param addr Address to read from.
    /// \param passive Don't trigger additional read behaviour
    /// \return Byte that has been read.
    uint8_t read(uint16_t addr, bool passive=false) final;

    /// Writes a value to the provided address.
    /// \param addr Address to write the value to.
    /// \param val Value to write.
    void write(uint16_t addr, uint8_t val) final;
};

class MissingCartridge {};
}  // namespace NES

#endif  // INC_2A03_BUS_H
