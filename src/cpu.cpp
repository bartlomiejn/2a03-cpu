#include <2a03/cpu.h>
#include <cstring>

#define lambda_u16(x) [&]() { return x(); }

void NES::CPU::start()
{
	A = 0x0;
	X = 0x0;
	Y = 0x0;
	S = 0x0;
	P = 0x4;
	memset(ram, 0xFF, sizeof(ram));
}

void NES::CPU::execute()
{
	switch (PC++)
	{
		// LDA
		case 0xAD: LD(&A, lambda_u16(abs)); break;
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

void NES::CPU::LD(uint8_t *reg, std::function<uint16_t(void)> mode_fn)
{

}

uint16_t NES::CPU::abs()
{

}