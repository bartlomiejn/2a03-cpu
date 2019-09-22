#include <iostream>
#include <2a03/cpu.h>
#include <2a03/cartridge/load.h>
#include <2a03/cartridge/mapper.h>

int main()
{
	NES::MemoryBus bus;
	NES::CPU cpu(bus);
	NES::iNESv1::Cartridge cartridge =
		NES::iNESv1::load("test/instr_test-v5/official_only.nes");
	
	NES::Mapper mapper(std::move(cartridge));
	bus.mapper = std::move(mapper);
	
	cpu.power();
	
	return 0;
}