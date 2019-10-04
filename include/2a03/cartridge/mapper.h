#ifndef INC_2A03_CARTRIDGE_H
#define INC_2A03_CARTRIDGE_H

#include <2a03/cartridge/ines.h>

namespace NES
{
namespace iNESv1
{
	enum MapperType
	{
		NROM = 0,
		MMC1 = 1
	};
	
	class Mapper
	{
	public:
		/// Initializes a Cartridge Mapper instance.
		/// \param cartridge Cartridge to use.
		explicit Mapper(NES::iNESv1::Cartridge &cartridge);
		
		/// Reads a byte of memory at the provided address.
		uint8_t read(uint16_t addr);
		
		/// Writes a byte of memory to the provided address.
		void write(uint16_t addr, uint8_t val);
	private:
		NES::iNESv1::Cartridge &cartridge; ///< Cartridge to map.
		
		/// Returns the ID of the mapper.
		uint8_t mapper_id();
	};
	
	class UnimplementedMapperType {};
}
}

#endif //INC_2A03_CARTRIDGE_H
