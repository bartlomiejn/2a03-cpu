#include <iostream>
#include <filesystem>
#include <2a03/cpu.h>
#include <2a03/cart/loader.h>

int main()
{
	NES::CPU cpu;
	
	std::filesystem::path opcodes_rom_p("");
	
	NES::iNESv1::load("../test/instr_test-v5/official_only.nes");
	
	cpu.power();
	
	return 0;
}