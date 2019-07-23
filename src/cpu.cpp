#include <2a03/cpu.h>
#include <cstring>
#include <iostream>

#define exec_lambda(x) [&]() { return x(); }

void NES::CPU::power_up()
{
	A = 0x0;
	X = 0x0;
	Y = 0x0;
	S = 0xFD;
	P.reg = 0x24;
	
	// TODO: Rest of power up logic
	// $4017 = $00 (frame irq enabled)
	// $4015 = $00 (all channels disabled)
	// $4000-$400F = $00 (not sure about $4010-$4013)
	// All 15 bits of noise channel LFSR = $0000[4]. The first time the LFSR
	// is clocked from the all-0s state, it will shift in a 1.
	
	// RAM state is not consistent on power up on a real machine, but we'll
	// clear it anyway.
	memset(ram, 0xFF, sizeof(ram));
}

void NES::CPU::reset()
{
	S -= 3;
	P.reg |= 0x04;
	
	// TODO: Rest of reset logic
	// APU was silenced ($4015 = 0)
	// APU triangle phase is reset to 0
}

void NES::CPU::execute()
{
	switch (PC)
	{
		// LDA
		case 0xA9:
			LD(&A, mode_imm); break;
		case 0xA5:
			LD(&A, mode_zp); break;
		case 0xB5:
			LD(&A, mode_zp_x); break;
		case 0xAD:
			LD(&A, mode_abs); break;
		case 0xBD:
			LD(&A, mode_abs_x); break;
		case 0xB9:
			LD(&A, mode_abs_y); break;
		default:
			std::cerr << "Unhandled opcode: " << std::hex << PC - 1
				<< std::endl;
	}
	PC++;
}

uint8_t NES::CPU::read(uint16_t addr)
{
	switch (addr)
	{
		case 0x0000 ... 0x1FFF:
			return ram[addr % 0x800];
		default:
			std::cerr << "Unhandled memory access: " << std::hex
				<< addr << std::endl;
	}
}

void NES::CPU::write(uint16_t addr, uint8_t val)
{
	switch (addr)
	{
		case 0x0000 ... 0x1FFF:
			ram[addr % 0x800] = val;
	}
}

void NES::CPU::LD(uint8_t *reg, NES::AddressingMode mode)
{
	*reg = get_param(mode);
}

uint8_t NES::CPU::get_param(NES::AddressingMode mode)
{
	uint16_t param_addr = 0x0;
	switch (mode)
	{
		// Absolute: A full 16-bit address is specified.
		case mode_abs:
			param_addr = (read(PC + 1) << 8) | read(PC);
			PC += 2;
			return read(param_addr);
			
		// Absolute Indexed with X: The value in X is added to the
		// specified address for a sum address. The value at the sum
		// address is used to perform the computation.
		case mode_abs_x:
			param_addr = ((read(PC + 1) << 8) | read(PC)) + X;
			PC += 2;
			return read(param_addr);
			
		// Absolute Indexed with Y: The value in Y is added to the
		// specified address for a sum address. The value at the sum
		// address is used to perform the computation.
		case mode_abs_y:
			param_addr = ((read(PC + 1) << 8) | read(PC)) + Y;
			PC += 2;
			return read(param_addr);
			
		// Immediate: The operand is a constant used directly to
		// perform the computation.
		case mode_imm:
			param_addr = PC;
			PC++;
			return read(param_addr);
			
		// Zero Page: A single byte specifies an address in the first
		// page of memory ($00xx, the zero page) and the byte at that
		// address is used to perform the computation.
		case mode_zp:
			param_addr = read(PC);
			PC++;
			return read(param_addr);
			
		// Zero Page indexed with X
		case mode_zp_x:
			param_addr = (read(PC) + X) % 0x100;
			PC++;
			return read(param_addr);
			
		// Zero Page indexed with Y
		case mode_zp_y:
			param_addr = (read(PC) + Y) % 0x100;
			PC++;
			return read(param_addr);
			
		default:
			std::cerr << "Invalid addressing mode: " << mode
				<< std::endl;
	}
	return read(param_addr);
}
