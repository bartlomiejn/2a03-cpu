#include <iostream>
#include <2a03/cpu.h>
#include <2a03/cart/loader.h>

int main()
{
	NES::CPU cpu;

	NES::iNESv1::load("test/instr_test-v5/official_only.nes");
	
	cpu.power();
	
	return 0;
}