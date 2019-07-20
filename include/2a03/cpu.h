#ifndef INC_2A03_CPU_H
#define INC_2A03_CPU_H

#include <cstdint>
#include <functional>
#include <2a03/ppu.h>
#include <2a03/apu.h>

namespace NES
{
	/// Status Register P union representation
	union StatusRegister
	{
		struct
		{
			uint8_t C : 1;
			uint8_t Z : 1;
			uint8_t I : 1;
			uint8_t D : 1;
			uint8_t B : 2; 	// Has no effect on CPU, but certain
					// instructions set it
			uint8_t V : 1;
			uint8_t N : 1;
		};
		uint8_t reg;
	};
	
	/// NTSC NES Ricoh 2a03 CPU emulator
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
		StatusRegister P;	/// Status register
		
		uint8_t ram[0x800]; 	/// RAM
		
		/// Starts the CPU.
		void start();
		
		/// Executes the next instruction.
		void execute();
	private:
		// Memory access
		
		/// Reads 8 bits of memory at the provided address.
		/// \param addr Address to read from
		/// \return Read data
		uint8_t read(uint16_t addr);
		
		/// Writes a value to the provided address.
		/// \param addr Address to write the value to.
		/// \param val Value to write.
		void write(uint16_t addr, uint8_t val);
		
		// Instructions
		
		/// Load register with memory.
		/// \param reg Register address to load memory to.
		/// \param mode_fn Addressing mode function to use.
		void LD(uint8_t *reg, std::function<uint16_t(void)> mode_fn);
		
		// Addressing mode functions
		
		/// Absolute:  A full 16-bit address is specified.
		/// \return Address to be accessed.
		uint16_t abs();
	};
}


#endif //INC_2A03_CPU_H
