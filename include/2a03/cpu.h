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
		mode_abs,	///< Absolute
		mode_abs_x,	///< Absolute indexed with X
		mode_abs_y,	///< Absolute indexed with Y
		mode_imm,	///< Immediate
		mode_zp,	///< Zero Page
		mode_zp_x,	///< Zero Page indexed with X
		mode_zp_y,	///< Zero Page indexed with Y
		mode_idx_ind_x,	///< Indexed indirect with X
		mode_ind_idx_y,	///< Indirect indexed with Y
		mode_ind	///< Indirect (used only with JMP)
	};
	
	/// Status Register P union representation
	union StatusRegister
	{
		struct
		{
			uint8_t C : 1; 	///< Carry
			uint8_t Z : 1; 	///< Zero
			uint8_t I : 1; 	///< Interrupt disable
			uint8_t D : 1; 	///< Decimal
			uint8_t B : 2; 	///< Has no effect on CPU, but certain
					///< instructions set it
			uint8_t V : 1;	///< Overflow
			uint8_t N : 1; 	///< Negative
		};
		uint8_t reg;
	};
	
	/// Ricoh 2a03 CPU emulator
	/// http://wiki.nesdev.com/w/index.php/CPU_registers
	class CPU
	{
	public:
		PPU ppu;		///< Picture Processing Unit
		APU apu;		///< Audio Processing Unit
		uint8_t A; 		///< Accumulator
		uint8_t X, Y; 		///< Index registers
		uint16_t PC;		///< Program counter
		uint8_t S;		///< Stack pointer
		StatusRegister P;	///< Status register
		uint8_t ram[0x800]; 	///< RAM
		
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
		
		/// Reads 16 bits of memory at the provided address.
		/// \param addr Address to read from
		/// \param is_zp_addr If it's a zero-page address, wrap the most
		/// -significant byte around zero-page
		/// \return 2 bytes that were read
		uint16_t read16(uint16_t addr, bool is_zp_addr = false);
		
		/// Writes a value to the provided address.
		/// \param addr Address to write the value to.
		/// \param val Value to write.
		void write(uint16_t addr, uint8_t val);
		
		// Addressing mode functions
		
		/// Retrieves the current instruction parameter based on
		/// the addressing mode and increments PC based on parameter
		/// length.
		/// \param mode Addressing mode to use.
		/// \return Parameter for current instruction.
		uint8_t get_param(AddressingMode mode);
		
		// Instructions
		
		/// Transfer program execution.
		/// \param mode Addressing mode to use.
		void JMP(AddressingMode mode);
		
		/// Load register `reg` with memory.
		/// \param reg Register address to load memory to.
		/// \param addr_fn Addressing mode to use.
		void LD(uint8_t *reg, AddressingMode mode);
		
		/// Transfer X to Stack
		void TXS();
	};
}

#endif //INC_2A03_CPU_H
