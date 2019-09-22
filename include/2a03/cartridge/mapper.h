#ifndef INC_2A03_CARTRIDGE_H
#define INC_2A03_CARTRIDGE_H

#include <2a03/cartridge/ines.h>

namespace NES
{
namespace iNESv1
{
	class Mapper
	{
	public:
		/// Initializes a Cartridge Mapper instance.
		/// \param cartridge Cartridge to use. Takes ownership of the
		/// cartridge instance.
		Mapper(NES::iNESv1::Cartridge cartridge);
		
		/// Reads a byte of memory at the provided address.
		uint8_t read(uint16_t addr);
		
		/// Reads 2 bytes of memory at the provided address.
		uint16_t read16(uint16_t addr);
		
		/// Writes a byte of memory to the provided address.
		void write(uint16_t addr, uint8_t val);
		
		// TODO: Mapper page banking.
	private:
		NES::iNESv1::Cartridge cartridge; ///< Cartridge to map.
	};
}
}

#endif //INC_2A03_CARTRIDGE_H
