#include <2a03/cartridge/mapper.h>

using namespace NES::iNESv1;

Mapper::Mapper(NES::iNESv1::Cartridge cartridge) :
	cartridge(std::move(cartridge))
{};

uint8_t Mapper::read(uint16_t addr)
{
	// TODO: Mapper memory access.
	return 0x0;
}

uint16_t Mapper::read16(uint16_t addr)
{
	// TODO: Mapper memory access.
	return 0x0;
}

void Mapper::write(uint16_t addr, uint8_t val)
{
	// TODO: Mapper memory access.
}