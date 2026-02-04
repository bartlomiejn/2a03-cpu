#ifndef INC_2A03_INES_H
#define INC_2A03_INES_H

#include <bitfield.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>

namespace NES {
namespace iNESv1 {
const uint16_t prg_rom_page_sz = 0x4000;  ///< PRG ROM page size - 16KB.
const uint16_t chr_rom_page_sz = 0x2000;  ///< CHR ROM page size - 8KB.
const uint16_t prg_ram_def_sz = 0x2000;   ///< PRG RAM default size - 8KB.
const uint16_t trainer_abs_sz = 0x200;    ///< Trainer absolute size - 512B.

bitfield_union(
    Byte6, uint8_t byte,

    bool mirror : 1;   ///< Mirroring  0 : horizontal (CIRAM A10 == PPU A11)
                       ///< 		1 : vertical (CIRAM A10 == PPU A10)
    bool prg_ram : 1;  ///< Contains battery-backed PRG RAM ($6000-$7FFF) or
                       ///< other persistent memory
    bool
        has_trainer : 1;  ///< 512-byte trainer at $7000-$71FF (before PRG data)
    bool ignore_mctrl : 1;  ///< Ignore mirroring control or above mirroring bit

    uint8_t nib_l : 4;  ///< Lower nibble of mapper number
);

// bitfield_union(
//     Byte6, uint8_t byte,
//     uint8_t nib_l : 4;      ///< Lower nibble of mapper number
//     bool ignore_mctrl : 1;  ///< Ignore mirroring control or above mirroring
//     bit bool
//         has_trainer : 1;  ///< 512-byte trainer at $7000-$71FF (before PRG
//         data)
//     bool prg_ram : 1;     ///< Contains battery-backed PRG RAM ($6000-$7FFF)
//     or
//                           ///< other persistent memory
//     bool mirror : 1;      ///< Mirroring  0 : horizontal (CIRAM A10 == PPU
//     A11)
//                           ///< 		1 : vertical (CIRAM A10 == PPU
//                           A10)
//);

bitfield_union(Byte7, uint8_t byte,
               bool vs_unisys : 1;  ///< Is VS Unisystem.
               bool pc_10 : 1;  ///< Is PlayChoice-10 (8KB hint screen stored
                                ///< after CHR data).
               uint8_t ines_v2 : 2;  ///< If equals 2, flags 8-15 are iNESv2.
               uint8_t nib_h : 4;    ///< Upper nibble of mapper number.
);

bitfield_union(Byte9, uint8_t byte,
               bool tv_sys : 2;  ///< 0 for NTSC, 2 for PAL, 1/3 dual compatible
               uint8_t RESERVED : 6;);

struct Header {
    Header(uint8_t prg_rom_pages, uint8_t chr_rom_pages, Byte6 flags_6,
           Byte7 flags_7, uint8_t prg_ram_sz, Byte9 flags_9)
        : prg_rom_banks(prg_rom_pages),
          chr_rom_banks(chr_rom_pages),
          flags_6(flags_6),
          flags_7(flags_7),
          prg_ram_banks(prg_ram_sz),
          flags_9(flags_9) {};

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
    /// \param _header iNESv1 file header
    /// \param _trainer_sz Trainer data size, if it's available.
    /// \param _prg_rom_sz Program code size.
    /// \param _chr_rom_sz Size of PPU data.
    /// \param _prg_ram_sz Program RAM size.
    Cartridge(Header _header, unsigned int _trainer_sz,
              unsigned int _prg_rom_sz, unsigned int _chr_rom_sz,
              unsigned int _prg_ram_sz)
        : header(_header),
          trainer(_trainer_sz, 0),
          prg_rom(_prg_rom_sz, 0),
          chr_rom(_chr_rom_sz, 0),
          prg_ram(_prg_ram_sz, 0) {};

    Header header;
    std::vector<uint8_t> trainer;
    std::vector<uint8_t> prg_rom;
    std::vector<uint8_t> chr_rom;
    std::vector<uint8_t> prg_ram;
};
}  // namespace iNESv1
}  // namespace NES

#endif  // INC_2A03_INES_H
