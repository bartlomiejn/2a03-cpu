#ifndef INC_2A03_CPU_H
#define INC_2A03_CPU_H

#include <cstdint>
#include <2a03/ppu.h>
#include <2a03/apu.h>

namespace NES
{
	enum AddrMode
	{
		case mode_imm,
		case mode_
	};
	
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
		
		uint8_t ram[0x800]; 	/// RAM
		
		/// Executes the next instruction.
		void execute();
	private:
		uint8_t read(uint16_t addr);
		void write(uint16_t addr, uint8_t val);
	};
}


#endif //INC_2A03_CPU_H
