#ifndef INC_2A03_INES_LOADER_H
#define INC_2A03_INES_LOADER_H

#include <string>
#include <2a03/cartridge/ines.h>

namespace NES
{
namespace iNESv1
{
	const unsigned int prg_rom_pagesz = 0x4000; ///< PRG ROM page size - 16KB.
	const unsigned int chr_rom_pagesz = 0x2000; ///< CHR ROM page size - 8KB.
	const unsigned int prg_ram_defsz = 0x2000;  ///< PRG RAM default size - 8KB.
	const unsigned int trainer_abssz = 0x200;   ///< Trainer absolute size - 512B.
	
	NES::iNESv1::Cartridge load(std::string &filename);
	
	class InvalidFile {};
	class InvalidMagicNumber {};
}
}

#endif