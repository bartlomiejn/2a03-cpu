#ifndef INC_2A03_INES_H
#define INC_2A03_INES_H

#include <2a03/utils/bitfield.h>

#include <algorithm>
#include <cstdint>
#include <memory>

namespace NES {
namespace iNESv1 {
const uint16_t prg_rom_page_sz = 0x4000;  ///< PRG ROM page size - 16KB.
const uint16_t chr_rom_page_sz = 0x2000;  ///< CHR ROM page size - 8KB.
const uint16_t prg_ram_def_sz = 0x2000;   ///< PRG RAM default size - 8KB.
const uint16_t trainer_abs_sz = 0x200;    ///< Trainer absolute size - 512B.

bitfield_union(
    Byte6, uint8_t byte,
    uint8_t nib_l : 4;      ///< Lower nibble of mapper number
    bool ignore_mctrl : 1;  ///< Ignore mirroring control or above mirroring bit
    bool
        has_trainer : 1;  ///< 512-byte trainer at $7000-$71FF (before PRG data)
    bool prg_ram : 1;     ///< Contains battery-backed PRG RAM ($6000-$7FFF) or
                          ///< other persistent memory
    bool mirror : 1;      ///< Mirroring  0 : horizontal (CIRAM A10 == PPU A11)
                          ///< 		1 : vertical (CIRAM A10 == PPU A10)
);

bitfield_union(Byte7, uint8_t byte,
               uint8_t nib_h : 4;    ///< Upper nibble of mapper number.
               uint8_t ines_v2 : 2;  ///< If equals 2, flags 8-15 are iNESv2.
               bool pc_10 : 1;  ///< Is PlayChoice-10 (8KB hint screen stored
                                ///< after CHR data).
               bool vs_unisys : 1;  ///< Is VS Unisystem.
);

bitfield_union(Byte9, uint8_t byte,
               uint8_t RESERVED : 7;
               bool tv_sys : 1;  ///< 0 for NTSC, 1 for PAL.
);

struct Header {
    Header(uint8_t prg_rom_pages, uint8_t chr_rom_pages, Byte6 flags_6,
           Byte7 flags_7, uint8_t prg_ram_sz, Byte9 flags_9)
        : prg_rom_banks(prg_rom_pages),
          chr_rom_banks(chr_rom_pages),
          flags_6(flags_6),
          flags_7(flags_7),
          prg_ram_banks(prg_ram_sz),
          flags_9(flags_9){};

    // Bytes 0-3 are `NES<EOF>` in ASCII
    uint8_t prg_rom_banks;  ///< Byte 4: PRG ROM bank count in 16KB units.
    uint8_t chr_rom_banks;  ///< Byte 5: CHR ROM bank count in 8KB units.
    Byte6 flags_6;          ///< Byte 6: Flags.
    Byte7 flags_7;          ///< Byte 7: Flags.
    uint8_t prg_ram_banks;  ///< Byte 8: PRG RAM bank count in 8KB units.
    Byte9 flags_9;          ///< Byte 9: Flags.
};

class Cartridge {
   public:
    /// iNESv1 cartridge image.
    /// \param header iNESv1 file header
    /// \param trainer_sz Trainer data size, if it's available.
    /// \param prg_rom_sz Program code size.
    /// \param chr_rom_sz Size of PPU data.
    /// \param prg_ram_sz Program RAM size.
    Cartridge(Header header, unsigned int trainer_sz, unsigned int prg_rom_sz,
              unsigned int chr_rom_sz, unsigned int prg_ram_sz)
        : header(header),
          trainer(std::make_unique<uint8_t[]>(trainer_sz)),
          prg_rom(std::make_unique<uint8_t[]>(prg_rom_sz)),
          chr_rom(std::make_unique<uint8_t[]>(chr_rom_sz)),
          prg_ram(std::make_unique<uint8_t[]>(prg_ram_sz)) {
        std::fill(trainer.get(), trainer.get() + trainer_sz, 0x0);
        std::fill(prg_rom.get(), prg_rom.get() + trainer_sz, 0x0);
        std::fill(chr_rom.get(), chr_rom.get() + trainer_sz, 0x0);
        std::fill(prg_ram.get(), prg_ram.get() + trainer_sz, 0x0);
    };

    Header header;
    std::unique_ptr<uint8_t[]> trainer;
    std::unique_ptr<uint8_t[]> prg_rom;
    std::unique_ptr<uint8_t[]> chr_rom;
    std::unique_ptr<uint8_t[]> prg_ram;
};
}  // namespace iNESv1
}  // namespace NES

#endif  // INC_2A03_INES_H
