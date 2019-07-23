#ifndef INC_2A03_CPU_H
#define INC_2A03_CPU_H

#include <cstdint>
#include <functional>
#include <2a03/ppu.h>
#include <2a03/apu.h>

namespace NES
{
	enum AddressingMode
	{
		mode_abs,	/// Absolute
		mode_abs_x,	/// Absolute indexed with X
		mode_abs_y,	/// Absolute indexed with Y
		mode_imm,	/// Immediate
		mode_zp,	/// Zero Page
		mode_zp_x,	/// Zero Page indexed with X
		mode_zp_y,	/// Zero Page indexed with Y
		mode_ind,	/// Indirect
		mode_ind_x,	/// Indexed indirect with X
		mode_ind_y	/// Indirect indexed with Y
	};
	
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
		/// \return Byte that was read
		uint8_t read(uint16_t addr);
		
		/// Writes a value to the provided address.
		/// \param addr Address to write the value to.
		/// \param val Value to write.
		void write(uint16_t addr, uint8_t val);
		
		// Instructions
		
		/// Load register `reg` with memory.
		/// \param reg Register address to load memory to.
		/// \param addr_fn Addressing mode function to use.
		void LD(uint8_t *reg, AddressingMode mode);
		
		// Addressing mode functions
		
		/// Retrieves the current instruction parameter based on
		/// the addressing mode and increments PC based on instruction
		/// length.
		/// \param mode Addressing mode to use.
		/// \return Parameter for current instruction.
		uint8_t get_param(AddressingMode mode);
	};
}

#endif //INC_2A03_CPU_H
