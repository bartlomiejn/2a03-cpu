#include <2a03/cart/loader.h>
#include <fstream>
#include <iostream>

using namespace NES::iNESv1;

bool is_magic_valid(std::string &fstring)
{
	std::string magic = fstring.substr(0, 4);
	return magic == "NES\x1A";
}

Header get_inesv1_header(std::string &fstring)
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

void NES::iNESv1::load(std::string &&filename)
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
	
	std::cout << filename << " header loaded successfully."
		<< std::endl;
}