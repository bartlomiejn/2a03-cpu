#include <2a03/cart/loader.h>
#include <fstream>
#include <iostream>

// TODO: Replace with Cartridge instance
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
	
	std::string magic = fstring.substr(0, 4);
	if (magic != "NES\x1A")
	{
		std::cerr << "Invalid magic number in " << filename
			<< " header. Probably not an iNES ROM." << std::endl;
		throw InvalidMagicNumber();
	}
	
	std::cout << "CartridgeLoader::load " << filename << " success." << std::endl;
}