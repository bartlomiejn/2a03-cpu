#ifndef INC_2A03_CARTRIDGE_H
#define INC_2A03_CARTRIDGE_H

#include <2a03/cartridge/ines.h>

namespace NES
{
	class Mapper
	{
	public:
		/// Creates a Cartridge Mapper instance.
		/// \param cartridge Cartridge to use.
		Mapper(NES::iNESv1::Cartridge cartridge);
		
	private:
		NES::iNESv1::Cartridge cartridge; ///< Cartridge to map.
	};
}

#endif //INC_2A03_CARTRIDGE_H
