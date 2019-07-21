#include <2a03/cpu.h>
#include <cstring>

#define exec_lambda(x) [&]() { return x(); }

void NES::CPU::power_up()
{
	A = 0x0;
	X = 0x0;
	Y = 0x0;
	S = 0xFD;
	P.reg = 0x24;
	
	// TODO: Rest of power up logic
	// $4017 = $00 (frame irq enabled)
	// $4015 = $00 (all channels disabled)
	// $4000-$400F = $00 (not sure about $4010-$4013)
	// All 15 bits of noise channel LFSR = $0000[4]. The first time the LFSR is clocked from the all-0s state, it will shift in a 1.
	
	// RAM state is not consistent on power up on a real machine, but we'll
	// clear it anyway
	memset(ram, 0xFF, sizeof(ram));
}

void NES::CPU::reset()
{
	S -= 3;
	P.reg |= 0x04;
	
	// TODO: Rest of reset logic
	// APU was silenced ($4015 = 0)
	// APU triangle phase is reset to 0
}

void NES::CPU::execute()
{
	switch (PC++)
	{
		// LDA
		case 0xAD: LD(&A, exec_lambda(abs)); break;
	}
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

void NES::CPU::LD(uint8_t *reg, std::function<uint16_t(void)> addr_fn)
{
	*reg = read(addr_fn());
}

uint16_t NES::CPU::abs()
{
	PC += 2;
	return (uint16_t)(PC - 2);
}