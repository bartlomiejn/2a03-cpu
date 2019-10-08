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
		NES::CPU &cpu;		///< CPU whose state is logged.
		NES::MemoryBus &bus;	///< Bus whose devices are logged.
		std::vector<std::string> logs; 	///< Contains logs of CPU state
						///< on each `log` call.
	private:
		/// Decodes an opcode into a readable string form.
		std::string decode(uint8_t opcode);
		
		/// Returns the addressing mode for an opcode, if it's
		/// applicable.
		std::optional<NES::AddressingMode> addr_mode(uint8_t opcode);
		
		/// Returns a templated string for provided mode.
		/// \param addr_mode Addressing mode to provide a template for.
		/// \return Templated string for provided mode with operand
		/// marked as `{{OPERAND}}`.
		std::string templ_for_mode(NES::AddressingMode addr_mode);
		
		/// Returns the operand length in bytes for provided mode.
		uint8_t operand_len(NES::AddressingMode addr_mode);
	};
}

#endif //INC_2A03_LOGGER_H
