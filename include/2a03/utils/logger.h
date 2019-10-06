#ifndef INC_2A03_LOGGER_H
#define INC_2A03_LOGGER_H

#include <2a03/cpu.h>

namespace NES
{
	class CPULogger
	{
	public:
		/// Instantiates a CPULogger instance which logs the state of
		/// the provided CPU.
		explicit CPULogger(NES::CPU &cpu);
		
		/// Logs a line with processor state.
		void log();
	};
}

#endif //INC_2A03_LOGGER_H
