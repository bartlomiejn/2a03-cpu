#include <iostream>
#include <string>
#include <unistd.h>
#include <chrono>
#include <ctime>
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

std::string gen_timepid_logname() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm lt = *std::localtime(&now_time);
    pid_t pid = getpid();
    std::stringstream time_s;
    time_s << std::put_time(&lt, "%Y-%m-%d_%H:%M:%S");
    return std::format("{}_{}.log", time_s.str(), pid);
}

void run_nestest()
{
	using namespace NES::iNESv1;
	
	NES::MemoryBus bus;
	NES::CPU cpu(bus);
	NES::CPULogger logger(cpu, bus);
    logger.instr_ostream = std::cerr;
    logger.log_filename = gen_timepid_logname();

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

int main()
{
	run_nestest();
	return 0;
}
