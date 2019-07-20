#include <2a03/cpu.h>

void NES::CPU::execute()
{
	
}

uint8_t NES::CPU::read(uint16_t addr)
{
	switch (addr)
	{
		case 0x0000 ... 0x1FFF:
			return ram[addr % 0x800];
	}
}

void NES::CPU::write(uint16_t addr, uint8_t val)
{
	switch (addr)
	{
		case 0x0000 ... 0x1FFF:
			ram[addr % 0x800] = val;
	}
}