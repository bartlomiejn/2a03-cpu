#include <iostream>
#include <string>
#include <2a03/cpu.h>
#include <2a03/cart/load.h>
#include <2a03/cart/mapper.h>
#include <2a03/utils/string.h>

namespace NES
{
	enum TestState
	{
		running = 0x80,
		reset_required = 0x81
	};
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

void run_instr_test_v5()
{
	using namespace NES::iNESv1;
	
	NES::MemoryBus bus;
	NES::CPU cpu(bus);
	
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
				
				const char runstate[3] = {
					(char)cartridge.prg_ram[0x1],
					(char)cartridge.prg_ram[0x2],
					(char)cartridge.prg_ram[0x3]
				};
				
				std::cout << "Run state: "
					  << runstate << std::endl;
				
				const char *outstr =
					(const char*)cartridge.prg_ram[0x4];
				
				std::cout << "Completed test with result code: "
					<< std::hex << static_cast<int>(status)
					<< ". " << std::endl;
				
				if (outstr)
					std::cout << "Output:" << outstr
						<< std::endl;
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