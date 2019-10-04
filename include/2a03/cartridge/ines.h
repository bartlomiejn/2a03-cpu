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
			bool has_trainer : 1;	///< 512-byte trainer at $7000-$71FF (before PRG data)
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
			uint8_t prg_rom_pages,
			uint8_t chr_rom_pages,
			Byte6 flags_6,
			Byte7 flags_7,
			uint8_t prg_ram_sz,
			Byte9 flags_9
		):
			prg_rom_pages(prg_rom_pages),
			chr_rom_pages(chr_rom_pages),
			flags_6(flags_6),
			flags_7(flags_7),
			prg_ram_sz(prg_ram_sz),
			flags_9(flags_9)
		{};
		
		// Bytes 0-3 are `NES<EOF>` in ASCII
		uint8_t prg_rom_pages;	///< Byte 4: PRG ROM pages count in 16KB units.
		uint8_t chr_rom_pages;	///< Byte 5: CHR ROM pages count in 8KB units.
		Byte6 flags_6;		///< Byte 6: Flags.
		Byte7 flags_7;		///< Byte 7: Flags.
		uint8_t prg_ram_sz;	///< Byte 8: PRG RAM pages count in 8KB units.
		Byte9 flags_9;		///< Byte 9: Flags.
	};
	
	class Cartridge
	{
	public:
		/// iNESv1 cartridge image.
		/// \param header iNESv1 file header
		/// \param trainer_sz Trainer data size, if it's available.
		/// \param prg_rom_sz Program code size.
		/// \param chr_rom_sz Size of PPU data.
		/// \param prg_ram_sz Program RAM size.
		Cartridge(
			Header header,
			unsigned int trainer_sz,
			unsigned int prg_rom_sz,
			unsigned int chr_rom_sz,
			unsigned int prg_ram_sz
		):
			header(header),
			trainer(std::make_unique<uint8_t[]>(trainer_sz)),
			prg_rom(std::make_unique<uint8_t[]>(prg_rom_sz)),
			chr_rom(std::make_unique<uint8_t[]>(chr_rom_sz)),
			prg_ram(std::make_unique<uint8_t[]>(prg_ram_sz))
		{};
		
		Header header;
		std::unique_ptr<uint8_t[]> trainer;
		std::unique_ptr<uint8_t[]> prg_rom;
		std::unique_ptr<uint8_t[]> chr_rom;
		std::unique_ptr<uint8_t[]> prg_ram;
	};
}
}

#endif //INC_2A03_INES_H
