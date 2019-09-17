#include <cstring>
#include <iostream>
#include <2a03/bus.h>

using namespace NES;

MemoryBus::MemoryBus()
{
	// Ram state is not consistent on a real machine.
	memset(ram, 0xFF, sizeof(ram));
}

uint8_t MemoryBus::read(uint16_t addr)
{
	switch (addr)
	{
		case 0x0000 ... 0x1FFF:
			// Ram has only 2KB, but it wraps around up to 0x1FFF
			// 0x0000 - 0x00FF is zero page
			// 0x0100 - 0x01FF is stack memory
			// 0x0200 - 0x07FF is RAM
			return ram[addr % 0x800];
		default:
			std::cerr << "Unhandled memory access: " << std::hex
				  << addr << std::endl;
			// TODO: Shouldn't we crash here?
			return 0;
	}
}

uint16_t MemoryBus::read16(uint16_t addr, bool is_zp)
{
	// If we know this is a zero-page addr, wrap the most-significant bit
	// around zero-page bounds
	uint16_t h_addr = is_zp ? ((addr + 1) % 0x100) : (addr + 1);
	return (read(h_addr) << 8) | read(addr);
}

void MemoryBus::write(uint16_t addr, uint8_t val)
{
	switch (addr)
	{
		case 0x0000 ... 0x1FFF:
			ram[addr % 0x800] = val;
		default:
			std::cerr << "Unhandled write to " << std::hex << addr
				  << " with value: " << val << std::endl;
	}
}
