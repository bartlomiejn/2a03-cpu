#include <iostream>
#include <2a03/cpu.h>
#include <2a03/cartridge/load.h>
#include <2a03/cartridge/mapper.h>

enum TestState
{
	state_running = 0x80,
	state_reset_required = 0x81
};

NES::MemoryBus bus;
NES::CPU cpu(bus);

void run_instr_test_v5()
{
	std::string test_file = "test/instr_test-v5/official_only.nes";
	
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
			case state_running:
				// Test is running
				break;
			case state_reset_required:
				std::cout << "Reset required. Performing."
					<< std::endl;
				cpu.reset();
				break;
			case 0x00 ... 0x7F: // Result code for completed test
				std::cout << "Completed test with result code: "
					<< std::hex << status << std::endl;
				break;
		}
	}
}

int main()
{
	run_instr_test_v5();
	return 0;
}