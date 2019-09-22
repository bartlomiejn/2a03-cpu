#include <2a03/cartridge/mapper.h>

NES::Mapper::Mapper(NES::iNESv1::Cartridge cartridge) :
	cartridge(std::move(cartridge))
{};

