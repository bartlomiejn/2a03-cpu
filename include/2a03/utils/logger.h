#ifndef INC_2A03_LOGGER_H
#define INC_2A03_LOGGER_H

#include <vector>
#include <2a03/cpu.h>
#include <2a03/bus.h>

namespace NES
{
	class CPULogger
	{
	public:
		/// Instantiates a CPULogger instance which logs the state of
		/// the provided CPU.
		/// \param cpu CPU instance to use.
		/// \param bus BUS instance to get opcode/operand data from.
		/// Should be the same instance as the one used by the CPU.
		CPULogger(NES::CPU &cpu, NES::MemoryBus &bus);
		
		bool is_cout_each_line_enabled; ///< Set to true to enable
						///< output to cout for each
						///< line.
		
		/// Logs a line with CPU state.
		void log();
	protected:
		NES::CPU &cpu;
		NES::MemoryBus &bus;
		std::vector<std::string> logs;
	private:
		/// Decodes an opcode into a readable string form.
		std::string decode(uint8_t opcode);
	};
}

#endif //INC_2A03_LOGGER_H
