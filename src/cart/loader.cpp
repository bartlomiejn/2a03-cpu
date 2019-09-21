#include <2a03/cart/loader.h>
#include <fstream>
#include <iostream>

using namespace NES::iNESv1;

static const int prg_rom_pagesz = 16384;	///< PRG ROM page size - 16KB.
static const int chr_rom_pagesz = 8192;		///< CHR ROM page size - 8KB.

static bool is_magic_valid(std::string &fstring)
{
	std::string magic = fstring.substr(0, 4);
	return magic == "NES\x1A";
}

static Header get_inesv1_header(std::string &fstring)
{
	auto prg_rom_sz = static_cast<uint8_t>(fstring[4]);
	auto chr_rom_sz = static_cast<uint8_t>(fstring[5]);
	Byte6 flags_6 = { .byte = static_cast<uint8_t>(fstring[6]) };
	Byte7 flags_7 = { .byte = static_cast<uint8_t>(fstring[7]) };
	auto prg_ram_sz = static_cast<uint8_t>(fstring[8]);
	Byte9 flags_9 = { .byte = static_cast<uint8_t>(fstring[9]) };
	return Header(
		prg_rom_sz, chr_rom_sz, flags_6, flags_7, prg_ram_sz, flags_9);
}

Cartridge NES::iNESv1::load(std::string &&filename)
{
	std::ifstream fstream(filename);
	if (!fstream)
	{
		std::cerr << "Invalid cartridge filename to load: " << filename
			<< "." << std::endl;
		throw InvalidFile();
	}
	
	std::string fstring(
		(std::istreambuf_iterator<char>(fstream)),
		std::istreambuf_iterator<char>());
	if (!is_magic_valid(fstring))
	{
		std::cerr << "Invalid magic number in " << filename
			<< " header. Probably not an iNES ROM." << std::endl;
		throw InvalidMagicNumber();
	}

	Header header = get_inesv1_header(fstring);
	
	std::unique_ptr<uint8_t[]> prg_rom_mem = std::make_unique<uint8_t[]>(new uint8_t[prg_rom_pagesz * header.prg_rom_pages]);
	uint8_t prg_rom_mem[prg_rom_pagesz * header.prg_rom_pages];
	uint8_t chr_rom_mem[prg_rom_pagesz * header.prg_rom_pages];
	
	Cartridge cart(header, prg_rom_mem, chr_rom_mem);
	
	std::cout << filename << " header loaded successfully."
		<< std::endl;
	
	return cart;
}