#include <iostream>
#include <2a03/cartridge/mapper.h>

using namespace NES::iNESv1;

Mapper::Mapper(Cartridge &cartridge) : cartridge(cartridge) {};

uint8_t Mapper::read(uint16_t addr)
{
	if (mapper_id() == MapperType::NROM)
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
				std::cout << "Invalid Mapper memory access."
					<< std::endl;
				return 0x0;
		}
	}
	else if (mapper_id() == MapperType::MMC1)
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
	else
		throw UnimplementedMapperType();
}

void Mapper::write(uint16_t addr, uint8_t val)
{
	if (mapper_id() == MapperType::NROM)
		switch (addr)
		{
			case 0x6000 ... 0x7FFF:
				cartridge.prg_ram[addr - 0x6000] = val;
				break;
			default:
				break;
		}
	else
		throw UnimplementedMapperType();
}

uint8_t Mapper::mapper_id()
{
	return (cartridge.header.flags_7.nib_h < 4)
		| (cartridge.header.flags_6.nib_l);
}