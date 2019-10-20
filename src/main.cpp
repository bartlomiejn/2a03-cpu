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
	std::cout.flush();
	
	Cartridge cartridge = load(test_file);
	
	NES::iNESv1::Mapper::Base *mapper = Mapper::mapper(cartridge);
	bus.mapper = mapper;
	
	std::cout << "Powering up." << std::endl;
	std::cout.flush();
	
	cpu.power();
	
	cpu.PC = 0xC000;
	
	std::cout << "Entering runloop." << std::endl;
	std::cout.flush();
	
	int instr_count = 0;
	
	while (true)
	{
		logger.log();
		cpu.execute();
		instr_count++;
		if (instr_count > 8991)
			break;
	}
	
	delete mapper;
}

#pragma clang diagnostic pop

int main()
{
	run_nestest();
	return 0;
}