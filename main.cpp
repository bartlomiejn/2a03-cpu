#include <iostream>
#include <2a03/cpu.h>

int main()
{
	NES::CPU cpu;
	cpu.execute(0xAD, 0x2001); // Load Accumulator with memory at $2001
	return 0;
}