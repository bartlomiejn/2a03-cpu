#ifndef INC_2A03_INES_LOADER_H
#define INC_2A03_INES_LOADER_H

#include <string>
#include <2a03/cart/ines.h>

namespace NES
{
namespace iNESv1
{
	NES::iNESv1::Cartridge load(std::string &filename);
	
	class InvalidFile {};
	class InvalidMagicNumber {};
}
}

#endif