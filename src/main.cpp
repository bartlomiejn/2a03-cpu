#include <iostream>
#include <string>
#include <2a03/cpu.h>
#include <2a03/cart/load.h>
#include <2a03/cart/mapper.h>
#include <2a03/utils/logger.h>

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

void run_nestest()
{
	using namespace NES::iNESv1;
	
	NES::MemoryBus bus;
	NES::CPU cpu(bus);
	NES::CPULogger logger(cpu, bus);
	
	std::string test_file = "../test/nestest/nestest.nes";
	
	std::cout << "Loading " << test_file << "." << std::endl;
	
	Cartridge cartridge = load(test_file);
	
	NES::iNESv1::Mapper::Base *mapper = Mapper::mapper(cartridge);
	bus.mapper = mapper;
	
	std::cout << "Powering up." << std::endl;
	
	cpu.power();
	
	cpu.PC = 0xC000;
	cpu.cycles = 7;
	
	std::cout << "Entering runloop." << std::endl;
	
	while (true)
	{
		logger.log();
		
		try { cpu.execute(); }
		catch (NES::InvalidOpcode err) { break; }
		
		if (bus.read(0x02) != 0x0) // Some sort of error occured:
		{
			std::cerr << "Nestest failure code: " << std::hex
				<< bus.read16(0x02) << "." << std::endl;
		}
	}
	
	std::cout << "Terminating." << std::endl;
	
	logger.save();
	
	delete mapper;
}

#pragma clang diagnostic pop

int main()
{
	run_nestest();
	return 0;
}