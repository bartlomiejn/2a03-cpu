#ifndef INC_2A03_CPU_H
#define INC_2A03_CPU_H

#include <cstdint>
#include <2a03/ppu.h>
#include <2a03/apu.h>

namespace NES
{
	/// NTSC NES Ricoh 2a03 CPU emulator
	///
	/// http://wiki.nesdev.com/w/index.php/CPU_registers
	class CPU
	{
	public:
		PPU ppu;		/// Picture Processing Unit
		APU apu;		/// Audio Processing Unit
		
		uint8_t A; 		/// Accumulator
		uint8_t X, Y; 		/// Index registers
		uint16_t PC;		/// Program counter
		uint8_t S;		/// Stack pointer
		uint8_t P;		/// Status register
		
		uint8_t mem[65535]; 	/// Memory
		
		/// Executes an instruction.
		///
		/// \param opcode Opcode of the instruction.
		void execute(uint8_t opcode);
		
		/// Executes an instruction.
		///
		/// \param opcode Opcode of the instruction.
		/// \param param Parameter of the instruction.
		void execute(uint8_t opcode, uint16_t param);
	};
}


#endif //INC_2A03_CPU_H
