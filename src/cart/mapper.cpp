#include <iostream>
#include <2a03/cartridge/mapper.h>

using namespace NES::iNESv1;

Mapper::Base *Mapper::mapper(NES::iNESv1::Cartridge &cartridge)
{
	uint8_t id = (cartridge.header.flags_7.nib_h < 4)
		     | (cartridge.header.flags_6.nib_l);
	switch (id)
	{
		case Mapper::type_NROM:
			return new Mapper::NROM(cartridge);
		case Mapper::type_MMC1:
			return new Mapper::MMC1(cartridge);
	}
}

// NROM

Mapper::NROM::NROM(Cartridge &cartridge) : Mapper::Base(cartridge) {}

uint8_t Mapper::NROM::read(uint16_t addr)
{
	switch (addr)
	{
		case 0x6000 ... 0x7FFF:
			return cartridge.prg_ram[addr - 0x6000];
		case 0x8000 ... 0xBFFF:
			// Low 16KB PRG ROM
			return cartridge.prg_rom[addr - 0x8000];
		case 0xC000 ... 0xFFFF:
			// High 16KB PRG ROM, or mirrored low if 16KB
			if (cartridge.header.prg_rom_pages == 1)
				return cartridge.prg_rom[addr - 0xC000];
			else
				return cartridge.prg_rom[addr - 0x8000];
		default:
			std::cout << "Invalid NROM Mapper memory access."
				  << std::endl;
			return 0x0;
	}
}

void Mapper::NROM::write(uint16_t addr, uint8_t val)
{
	switch (addr)
	{
		case 0x6000 ... 0x7FFF:
			cartridge.prg_ram[addr - 0x6000] = val;
			break;
		default:
			break;
	}
}

// MMC1

Mapper::MMC1::MMC1(Cartridge &cartridge) : Mapper::Base(cartridge) {}

uint8_t Mapper::MMC1::read(uint16_t addr)
{
	switch (addr)
	{
		case 0x6000 ... 0x7FFF:
			return cartridge.prg_ram[addr - 0x6000];
		case 0x8000 ... 0xBFFF:
			// First bank 16KB PRG ROM or switchable
			return cartridge.prg_rom[addr - 0x8000];
		case 0xC000 ... 0xFFFF:
			// Last bank 16KB PRG ROM or switchable
			if (cartridge.header.prg_rom_pages == 1)
				return cartridge.prg_rom[addr - 0xC000];
			else
				return cartridge.prg_rom[addr - 0x8000];
		default:
			std::cout << "Invalid Mapper memory access."
				  << std::endl;
			return 0x0;
	}
}

void Mapper::MMC1::write(uint16_t addr, uint8_t val)
{
	switch (addr)
	{
		case 0x6000 ... 0x7FFF:
			cartridge.prg_ram[addr - 0x6000] = val;
			break;
		default:
			break;
	}
}