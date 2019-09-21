#include <iostream>
#include <2a03/cpu.h>
#include <2a03/cart/loader.h>

int main()
{
	NES::MemoryBus bus;
	NES::CPU cpu(bus);
	
	NES::iNESv1::Cartridge cart =
		NES::iNESv1::load("test/instr_test-v5/official_only.nes");
	
	cpu.power();
	
	return 0;
}