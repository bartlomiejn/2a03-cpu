#include <2a03/cpu.h>
#include <cstring>
#include <iostream>

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
		// JMP
		case 0x4C:
			JMP(mode_abs); break;
		case 0x6C:
			JMP(mode_ind); break;
			
		// LDA / LDX / LDY
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
		case 0xA1:
			LD(&A, mode_idx_ind_x); break;
		case 0xB1:
			LD(&A, mode_ind_idx_y); break;
		case 0xA2:
			LD(&X, mode_imm); break;
		case 0xA6:
			LD(&X, mode_zp); break;
		case 0xB6:
			LD(&X, mode_zp_y); break;
		case 0xAE:
			LD(&X, mode_abs); break;
		case 0xBE:
			LD(&X, mode_abs_y); break;
		case 0xA0:
			LD(&Y, mode_imm); break;
		case 0xA4:
			LD(&Y, mode_zp); break;
		case 0xB4:
			LD(&Y, mode_zp_x); break;
		case 0xAC:
			LD(&Y, mode_abs); break;
		case 0xBC:
			LD(&Y, mode_abs_x); break;
			
		// TXS / TSX
		case 0x9A:
			T(&X, &S); break;
		case 0xBA:
			T(&S, &X); break;
			
		// PHA / PHP
		case 0x48:
			PH(A); break;
		case 0x08:
			PH(P.reg); break;
			
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
			// Ram has only 2KB, but it wraps around up to 0x1FFF
			return ram[addr % 0x800];
		default:
			std::cerr << "Unhandled memory access: " << std::hex
				<< addr << std::endl;
	}
}

uint16_t NES::CPU::read16(uint16_t addr, bool is_zp_addr)
{
	// If we know this is a zero-page addr, wrap the most-significant bit
	// around zero-page bounds
	uint16_t h_addr = is_zp_addr ? ((addr + 1) % 0x100) : (addr + 1);
	return (read(h_addr) << 8) | read(addr);
}

void NES::CPU::write_to(uint16_t addr, uint8_t val)
{
	switch (addr)
	{
		case 0x0000 ... 0x1FFF:
			ram[addr % 0x800] = val;
		default:
			std::cerr << "Unhandled write to " << std::hex << addr
				<< " with value: " << val << std::endl;
	}
}

uint8_t NES::CPU::get_param(NES::AddressingMode mode)
{
	uint16_t addr = 0x0;
	switch (mode)
	{
		case mode_abs:
			addr = read16(PC); PC += 2; break;
		case mode_abs_x:
			addr = read16(PC) + X; PC += 2; break;
		case mode_abs_y:
			addr = read16(PC) + Y; PC += 2; break;
		case mode_imm:
			addr = PC; PC++; break;
		case mode_zp:
			addr = read(PC); PC++; break;
		case mode_zp_x:
			addr = (read(PC) + X) % 0x100; PC++; break;
		case mode_zp_y:
			addr = (read(PC) + Y) % 0x100; PC++; break;
		case mode_idx_ind_x:
			addr = read16((read(PC) + X) % 0x100, true); PC++; break;
		case mode_ind_idx_y:
			addr = read16(read(PC), true) + Y; PC++; break;
		case mode_ind:
		default:
			std::cerr << "Invalid addressing mode: " << mode
				<< std::endl;
	}
	return read(addr);
}

void NES::CPU::JMP(NES::AddressingMode mode)
{
	switch (mode)
	{
		case mode_ind:
			// TODO: Implement indirect jump quirk
			// AN INDIRECT JUMP MUST NEVER USE A
			// VECTOR BEGINNING ON THE LAST BYTE
			// OF A PAGE
			// For example if address $3000 contains $40, $30FF
			// contains $80, and $3100 contains $50, the result of
			// JMP ($30FF) will be a transfer of control to $4080
			// rather than $5080 as you intended i.e. the 6502 took
			// the low byte of the address from $30FF and the high
			// byte from $3000.
			PC = read16(read16(PC)); break;
		case mode_abs:
			PC = read16(PC);
		default:
			std::cerr << "Invalid addressing mode for JMP: " << mode
				  << std::endl;
	}
}

void NES::CPU::LD(uint8_t *reg, NES::AddressingMode mode)
{
	*reg = get_param(mode);
}

void NES::CPU::T(uint8_t *reg_from, uint8_t *reg_to)
{
	*reg_to = *reg_from;
}

void NES::CPU::PH(uint8_t value)
{
	write_to(S, value);
	S++;
}
