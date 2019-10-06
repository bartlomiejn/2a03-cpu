#include <string>
#include <sstream>
#include <iomanip>
#include <2a03/utils/logger.h>

NES::CPULogger::CPULogger(NES::CPU &cpu, NES::MemoryBus &bus) :
	cpu(cpu),
	bus(bus),
	logs(),
	is_cout_each_line_enabled(true)
{}

void NES::CPULogger::log()
{
	std::string line;
	std::stringstream ss;
	
	// PC as a 4-char wide hex string.
	ss << std::setfill('0') << std::setw(4) << std::hex << (int)cpu.PC;
	line += std::string(ss.str());
	ss.str(std::string());
	
	line += "  ";
	
	// Current opcode as hex.
	ss << std::setfill('0') << std::setw(2) << std::hex
		<< (int)bus.read(cpu.PC);
	line += std::string(ss.str());
	ss.str(std::string());
	
	line += " ";
	
	logs.push_back(line);
}