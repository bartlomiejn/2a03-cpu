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
		
		/// Dumps the accumulated log to a file.
		void save();
		
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
		std::optional<NES::AddressingMode> addr_mode_for_op(
			uint8_t opcode);
		
		/// Returns a templated string for provided mode.
		/// \param addr_mode Addressing mode to provide a template for.
		/// \return Templated string for provided mode with optional
		/// operand marked as `{{OPERAND}}` and optional target marked
		/// as `{{TARGET}}`.
		std::string templ_for_mode(NES::AddressingMode addr_mode);
		
		/// Returns the operand length in bytes for provided mode.
		uint8_t operand_len(NES::AddressingMode addr_mode);
		
		/// If there is a target for a specified mode, returns
		/// the amount of bytes to be printed.
		uint8_t target_len(NES::AddressingMode addr_mode);
		
		/// Retrieve target value for specified addressing mode.
		uint16_t target_value(NES::AddressingMode addr_mode);
		
		/// Returns true if provided `opcode` is part of the official
		/// instruction set.
		bool is_opcode_legal(uint8_t opcode);
	};
}

#endif //INC_2A03_LOGGER_H
