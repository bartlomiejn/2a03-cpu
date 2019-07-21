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
			uint8_t C : 1; 	/// Carry
			uint8_t Z : 1; 	/// Zero
			uint8_t I : 1; 	/// Interrupt disable
			uint8_t D : 1; 	/// Decimal
			uint8_t B : 2; 	/// Has no effect on CPU, but certain
					/// instructions set it
			uint8_t V : 1;	/// Overflow
			uint8_t N : 1; 	/// Negative
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
		void power_up();
		
		/// Resets the CPU.
		void reset();
		
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
		
		/// Load register `reg` with memory.
		/// \param reg Register address to load memory to.
		/// \param addr_fn Addressing mode function to use.
		void LD(uint8_t *reg, std::function<uint16_t(void)> addr_fn);
		
		// Addressing mode functions
		
		/// Absolute: A full 16-bit address is specified.
		/// \return Address to be accessed.
		uint16_t abs();
		
		/// Absolute Indexed with X: The value in X is added to the
		/// specified address for a sum address. The value at the sum
		/// address is used to perform the computation.
		/// \return Address to be accessed.
		uint16_t abs_x();
		
		/// Absolute Indexed with Y: The value in Y is added to the
		/// specified address for a sum address. The value at the sum
		/// address is used to perform the computation.
		/// \return Address to be accessed.
		uint16_t abs_y();
		
		/// Immediate: The operand is used directly to perform the
		/// computation.
		/// \return Address to be accessed.
		uint16_t immediate();
		
		/// Zero Page: A single byte specifies an address in the first
		/// page of memory ($00xx, the zero page) and the byte at that
		/// address is used to perform the computation.
		/// \return Address to be accessed.
		uint16_t zp();
	};
}


#endif //INC_2A03_CPU_H
