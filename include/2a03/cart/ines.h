#ifndef INC_2A03_INES_H
#define INC_2A03_INES_H

#include <cstdint>
#include <memory>

namespace NES
{
namespace iNESv1
{
	union Byte6
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
	
	union Byte7
	{
		struct
		{
			uint8_t nib_h : 4;	///< Upper nibble of mapper number.
			uint8_t ines_v2 : 2;	///< If equals 2, flags 8-15 are iNESv2.
			bool pc_10 : 1;		///< Is PlayChoice-10 (8KB hint screen stored after CHR data).
			bool vs_unisys : 1;	///< Is VS Unisystem.
		};
		uint8_t byte;
	};
	
	union Byte9
	{
		struct
		{
			uint8_t reserved : 7; 	///< Reserved.
			bool tv_sys : 1; 	///< 0 for NTSC, 1 for PAL.
		};
		uint8_t byte;
	};
	
	struct Header
	{
		Header(
			uint8_t prg_rom_sz,
			uint8_t chr_rom_sz,
			Byte6 flags_6,
			Byte7 flags_7,
			uint8_t prg_ram_sz,
			Byte9 flags_9
		){
			this->prg_rom_sz = prg_rom_sz;
			this->chr_rom_sz = chr_rom_sz;
			this->flags_6 = flags_6;
			this->flags_7 = flags_7;
			this->prg_ram_sz = prg_ram_sz;
			this->flags_9 = flags_9;
		};
		
		// Bytes 0-3 are `NES<EOF>` in ASCII
		uint8_t prg_rom_sz;	///< Byte 4: PRG ROM pages count in 16KB units.
		uint8_t chr_rom_sz;	///< Byte 5: CHR ROM pages count in 8KB units.
		Byte6 flags_6;		///< Byte 6: Flags.
		Byte7 flags_7;		///< Byte 7: Flags.
		uint8_t prg_ram_sz;	///< Byte 8: PRG RAM pages count in 8KB units. 0 equals 8KB.
		Byte9 flags_9;		///< Byte 9: Flags.
	};
	
	class Cartridge
	{
	public:
		/// iNESv1 cartridge image.
		/// \param header iNESv1 file header
		/// \param prg_rom Program code to be executed.
		/// \param chr_rom ROM which is accessible by the PPU.
		Cartridge(
			Header header,
			std::unique_ptr<uint8_t[]> prg_rom,
			std::unique_ptr<uint8_t[]> chr_rom);
		
		Header header;
		std::unique_ptr<uint8_t[]> prg_rom;
		std::unique_ptr<uint8_t[]> chr_rom;
	};
}
}

#endif //INC_2A03_INES_H
