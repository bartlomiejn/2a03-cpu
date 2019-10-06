#include <fstream>
#include <iostream>
#include <2a03/cart/load.h>

using namespace NES::iNESv1;

/// Checks if magic number is valid. Increments the input iterator by 4 bytes.
static bool is_magic_valid(std::string::iterator &iter)
{
	std::string magic;
	for (int i = 0; i < 4; i++)
	{
		magic += *iter;
		iter++;
	}
	return magic == "NES\x1A";
}

/// Generates an iNESv1 header. Increments the input iterator by 6 bytes.
static Header get_inesv1_header(std::string::iterator &iter)
{
	auto prg_rom_sz = static_cast<uint8_t>(*iter);
	iter++;
	auto chr_rom_sz = static_cast<uint8_t>(*iter);
	iter++;
	Byte6 flags_6 = { .byte = static_cast<uint8_t>(*iter) };
	iter++;
	Byte7 flags_7 = { .byte = static_cast<uint8_t>(*iter) };
	iter++;
	auto prg_ram = static_cast<uint8_t>(*iter);
	auto prg_ram_sz = prg_ram != 0
		? prg_ram * prg_ram_def_sz 	// If not 0 then calculate size
		: prg_ram_def_sz; 		// If 0 then 8KB
	iter++;
	Byte9 flags_9 = { .byte = static_cast<uint8_t>(*iter) };
	iter++;
	return Header(
		prg_rom_sz, chr_rom_sz, flags_6, flags_7, prg_ram_sz, flags_9);
}

Cartridge NES::iNESv1::load(std::string &filename)
{
	// Attempt to open the file
	std::ifstream fstream(filename);
	if (!fstream)
	{
		std::cerr << "Invalid cartridge filename to load: " << filename
			<< "." << std::endl;
		throw InvalidFile();
	}
	
	// Get the file contents and create iterator
	std::string fstring(
		(std::istreambuf_iterator<char>(fstream)),
		std::istreambuf_iterator<char>());
	fstream.close();
	std::string::iterator fstr_iter = fstring.begin();
	
	// Verify the iNES magic number
	if (!is_magic_valid(fstr_iter))
	{
		std::cerr << "Invalid magic number in " << filename
			<< " header. Probably not an iNES ROM." << std::endl;
		throw InvalidMagicNumber();
	}

	// Generate header based on provided file
	Header header = get_inesv1_header(fstr_iter);
	
	// Generate cartridge object based on the header
	unsigned int trainer_sz = header.flags_6.has_trainer ? trainer_abs_sz : 0;
	unsigned int prg_rom_sz = prg_rom_page_sz * header.prg_rom_banks;
	unsigned int chr_rom_sz = chr_rom_page_sz * header.chr_rom_banks;
	unsigned int prg_ram_sz = header.prg_ram_banks;
	Cartridge cart(header, trainer_sz, prg_rom_sz, chr_rom_sz, prg_ram_sz);

	// Advance to byte 16 which is directly after the header
	fstr_iter += 6;
	
	// If trainer is available, copy trainer memory first
	if (header.flags_6.has_trainer)
	{
		for (int i = 0; i < trainer_sz; i++)
		{
			cart.trainer[i] = static_cast<uint8_t>(*fstr_iter);
			fstr_iter++;
		}
	}
	
	// Copy PRG ROM
	for (int i = 0; i < prg_rom_sz; i++)
	{
		cart.prg_rom[i] = static_cast<uint8_t>(*fstr_iter);
		fstr_iter++;
	}
	
	// Copy CHR ROM
	for (int i = 0; i < chr_rom_sz; i++)
	{
		cart.chr_rom[i] = static_cast<uint8_t>(*fstr_iter);
		fstr_iter++;
	}
	
	std::cout << filename << " ROM loaded successfully."
		<< std::endl;
	
	return cart;
}