#include <iostream>
#include <string>
#include <2a03/cpu.h>
#include <2a03/cartridge/load.h>
#include <2a03/cartridge/mapper.h>

namespace NES
{
	enum TestState
	{
		running = 0x80,
		reset_required = 0x81
	};
}

NES::MemoryBus bus;
NES::CPU cpu(bus);

std::string str_to_hex(const std::string &input)
{
	static const char* const lut = "0123456789ABCDEF";
	size_t len = input.length();
	
	std::string output;
	output.reserve(2 * len);
	for (size_t i = 0; i < len; ++i)
	{
		const unsigned char c = input[i];
		output.push_back(lut[c >> 4]);
		output.push_back(lut[c & 15]);
	}
	return output;
}

void run_instr_test_v5()
{
	std::string test_file = "../test/instr_test-v5/official_only.nes";
	
	std::cout << "Loading " << test_file << "." << std::endl;
	
	NES::iNESv1::Cartridge cartridge = NES::iNESv1::load(test_file);
	NES::iNESv1::Mapper mapper(cartridge);
	bus.mapper = std::optional { std::ref(mapper) };
	
	std::cout << "Powering up." << std::endl;
	
	cpu.power();
	
	std::cout << "Entering runloop." << std::endl;
	
	while (true)
	{
		cpu.execute();
		uint8_t status = cartridge.prg_ram[0x0];
		switch (status)
		{
			case NES::running:
				// Test is running
				break;
			case NES::reset_required:
				std::cout << "Reset required. Performing."
					<< std::endl;
				cpu.reset();
				break;
			case 0x00 ... 0x7F:
				std::string runstate(
					(const char*)cartridge.prg_ram[0x1], 3);
				std::cout << "Run state: "
					<< str_to_hex(runstate) << std::endl;
				
				std::string out_str(
					(const char*)cartridge.prg_ram[0x4]);
				std::cout << "Completed test with result code: "
					<< std::hex << status << ". "
					<< "Output:" << std::endl
					<< out_str << std::endl;
				break;
		}
	}
}

int main()
{
	run_instr_test_v5();
	return 0;
}