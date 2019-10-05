#include <iostream>
#include <string>
#include <2a03/cpu.h>
#include <2a03/cartridge/load.h>
#include <2a03/cartridge/mapper.h>
#include <2a03/utils/string.h>

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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

void run_instr_test_v5()
{
	using namespace NES::iNESv1;
	
	std::string test_file = "../test/instr_test-v5/official_only.nes";
	
	std::cout << "Loading " << test_file << "." << std::endl;
	
	Cartridge cartridge = load(test_file);
	NES::iNESv1::Mapper::Base *mapper = Mapper::mapper(cartridge);
	bus.mapper = mapper;
	
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
			default:
				if (status >= 0x80)
					break;
				
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
		}
	}
	
	delete mapper;
}

#pragma clang diagnostic pop

int main()
{
	run_instr_test_v5();
	return 0;
}