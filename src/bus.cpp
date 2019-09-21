#include <cstring>
#include <iostream>
#include <2a03/bus.h>

using namespace NES;

MemoryBus::MemoryBus()
{
	// Ram state is not consistent on a real machine.
	std::fill(ram.begin(), ram.end(), 0xFF);
	std::fill(cartridge.begin(), cartridge.end(), 0xFF);
}

uint8_t MemoryBus::read(uint16_t addr)
{
	switch (addr)
	{
		// Internal RAM
		case 0x0000 ... 0x1FFF:
			// 0x0000 - 0x00FF is zero page
			// 0x0100 - 0x01FF is stack memory
			// 0x0200 - 0x07FF is RAM
			return ram[addr % 0x800];
			
		// PPU registers
		case 0x2000 ... 0x3FFF:
			// TODO: Implement PPU, access with (addr % 8)
			std::cerr << "PPU register access at " << std::hex
				  << addr << "." << std::endl;
			return 0xFF;
			
		// APU registers
		case 0x4000 ... 0x4017:
			// TODO: Implement APU
			std::cerr << "APU register access at " << std::hex
				  << addr << "." << std::endl;
			return 0xFF;
			
		// CPU test mode APU/IO functionality (disabled)
		case 0x4018 ... 0x401F:
			std::cerr << "CPU test mode memory access at "
				<< std::hex << addr << "." << std::endl;
			return 0x0;
			
		// Cartridge space: PRG ROM, PRG RAM, mapper registers
		case 0x6000 ... 0xFFFF:
			return cartridge[addr - 0x6000];
			
		default:
			// TODO: Shouldn't we crash here really?
			std::cerr << "Unhandled memory access: " << std::hex
				  << addr << std::endl;
			return 0x0;
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
