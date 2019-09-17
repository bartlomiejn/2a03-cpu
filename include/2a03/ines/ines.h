#ifndef INC_2A03_INES_H
#define INC_2A03_INES_H

#include <cstdint>
#include <memory>

namespace INES
{
namespace v1
{
	union Flags6
	{
		struct
		{
			uint8_t nib_l : 4;	///< Lower nibble of mapper number
			bool ignore_mctrl : 1; 	///< Ignore mirroring control or above mirroring bit
			bool trainer : 1;	///< 512-byte trainer at $7000-$71FF (before PRG data)
			bool prg_ram : 1;	///< Contains battery-backed PRG RAM ($6000-$7FFF) or other persistent memory
			bool mirror : 1;	///< Mirroring  0 : horizontal (CIRAM A10 == PPU A11)
						///< 		1 : vertical (CIRAM A10 == PPU A10)
		};
		uint8_t byte;
	};
	
	union Flags7
	{
		struct
		{
			uint8_t nib_h : 4;	///< Upper nibble of mapper number
			uint8_t ines_v2 : 2;	///< If equals 2, flags 8-15 are iNESv2
			bool pc_10 : 1;		///< PlayChoice-10 (8KB hint screen stored after CHR data)
			bool vs_unisys : 1;	///< VS Unisystem
		};
		uint8_t byte;
	};
	
	union Flags9
	{
		struct
		{
			uint8_t reserved : 7; 	///< Reserved
			bool tv_sys : 1; 	///< 0 for NTSC, 1 for PAL.
		};
		uint8_t byte;
	};
	
	struct Header
	{
		// Bytes 0-3 are `NES<EOF>` in ASCII
		uint8_t prg_rom_sz;	///< Byte 4: PRG ROM size in 16KB units.
		uint8_t chr_rom_sz;	///< Byte 5: CHR ROM size in 8KB units.
		Flags6 flags6;		///< Byte 6: Flags.
		Flags7 flags7;		///< Byte 7: Flags.
		uint8_t prg_ram_sz;	///< Byte 8: PRG RAM size in 8KB units. 0 equals 8KB.
		Flags9 flags9;		///< Byte 9: Flags.
	};
	
	class Cartridge
	{
	public:
		Header header[16];
		std::unique_ptr<uint8_t[]> prg_rom;
		std::unique_ptr<uint8_t[]> chr_rom;
	};
}
}

#endif //INC_2A03_INES_H
