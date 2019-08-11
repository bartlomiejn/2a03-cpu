#include <2a03/cpu.h>
#include <cstring>
#include <iostream>

using namespace NES;

void CPU::power_up()
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

void CPU::reset()
{
	S -= 3;
	P.reg |= 0x04;
	
	// TODO: Rest of reset logic
	// APU was silenced ($4015 = 0)
	// APU triangle phase is reset to 0
}

void CPU::execute()
{
	switch (read(PC++))
	{
		// Control transfer
		
		// JMP
		case 0x4C:
			JMP(mode_abs); break;
		case 0x6C:
			JMP(mode_ind); break;
		// JSR
		case 0x20:
			JSR(); break;
		// RTS
		case 0x60:
			RTS(); break;
			
		// Arithmetic / logical
		
		// ORA
		case 0x09:
			ORA(mode_imm); break;
		case 0x05:
			ORA(mode_zp); break;
		case 0x15:
			ORA(mode_zp_x); break;
		case 0x0D:
			ORA(mode_abs); break;
		case 0x1D:
			ORA(mode_abs_x); break;
		case 0x19:
			ORA(mode_abs_y); break;
		case 0x01:
			ORA(mode_idx_ind_x); break;
		case 0x11:
			ORA(mode_ind_idx_y); break;
		// ROL / ROR
		case 0x2A:
			ROLA(); break;
		case 0x26:
			ROL(mode_zp); break;
		case 0x36:
			ROL(mode_zp_x); break;
		case 0x2E:
			ROL(mode_abs); break;
		case 0x3E:
			ROL(mode_abs_x); break;
		case 0x6A:
			RORA(); break;
		case 0x66:
			ROR(mode_zp); break;
		case 0x76:
			ROR(mode_zp_x); break;
		case 0x6E:
			ROR(mode_abs); break;
		case 0x7E:
			ROR(mode_abs_x); break;
		
		// Load / store
		
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
		case 0xA1:
			LD(&A, mode_idx_ind_x); break;
		case 0xB1:
			LD(&A, mode_ind_idx_y); break;
		// LDX
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
		// LDY
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
		
		// STA
		case 0x85:
			ST(A, mode_zp); break;
		case 0x95:
			ST(A, mode_zp_x); break;
		case 0x8D:
			ST(A, mode_abs); break;
		case 0x9D:
			ST(A, mode_abs_x); break;
		case 0x99:
			ST(A, mode_abs_y); break;
		case 0x81:
			ST(A, mode_idx_ind_x); break;
		case 0x91:
			ST(A, mode_ind_idx_y); break;
		// STX
		case 0x86:
			ST(X, mode_zp); break;
		case 0x96:
			ST(X, mode_zp_y); break;
		case 0x8E:
			ST(X, mode_abs); break;
		// STY
		case 0x84:
			ST(Y, mode_zp); break;
		case 0x94:
			ST(Y, mode_zp_x); break;
		case 0x8C:
			ST(Y, mode_abs); break;
		
		// Stack
		
		// TXS / TSX
		case 0x9A:
			T(&X, &S); break;
		case 0xBA:
			T(&S, &X); break;
		// PHA / PHP
		case 0x48:
			PH(A); break;
		case 0x08:
			// PHP sets P.B to 0x3
			PH(P.reg, true); break;
		// PLA / PLP
		case 0x68:
			PL(&A); break;
		case 0x28:
			PL(&P.reg); break;
			
		// Others
		
		// NOP
		case 0xEA:
			break;
		
		default:
			std::cerr << "Unhandled opcode: " << std::hex
				<< read(PC - 1) << std::endl;
	}
}

uint8_t CPU::read(uint16_t addr)
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

uint16_t CPU::read16(uint16_t addr, bool is_zp)
{
	// If we know this is a zero-page addr, wrap the most-significant bit
	// around zero-page bounds
	uint16_t h_addr = is_zp ? ((addr + 1) % 0x100) : (addr + 1);
	return (read(h_addr) << 8) | read(addr);
}

void CPU::write_to(uint16_t addr, uint8_t val)
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

uint8_t CPU::get_param(AddressingMode mode)
{
	return read(param_addr(mode));
}

uint16_t CPU::param_addr(AddressingMode mode)
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
	return addr;
}

// Auxiliary

uint8_t CPU::rot_l(uint8_t value)
{
	return (value << 1) | (value >> (sizeof(value) * 8 - 1));
}

uint8_t CPU::rot_r(uint8_t value)
{
	return (value >> 1) | (value << (sizeof(value) * 8 - 1));
}

// Control transfer

void CPU::JMP(AddressingMode mode)
{
	switch (mode)
	{
		case mode_ind:
			// TODO: Implement indirect jump quirk
			// AN INDIRECT JUMP MUST NEVER USE A VECTOR BEGINNING ON
			// THE LAST BYTE OF A PAGE
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

void CPU::JSR()
{
	uint16_t return_addr = (uint16_t)(PC + 1);
	PH((uint8_t)return_addr >> 8);
	PH((uint8_t)return_addr);
	JMP(mode_abs);
}

void CPU::RTS()
{
	uint8_t l_addr, h_addr;
	PL(&l_addr);
	PL(&h_addr);
	PC = h_addr << 8 | l_addr;
}

// Arithmetic / logical

void CPU::ORA(AddressingMode mode)
{
	uint8_t param = get_param(mode);
	A |= param;
	P.Z = A == 0;
	P.N = A >> 7;
}

// TODO: Add N, Z, C to ROL / ROR ops
void CPU::ROLA()
{
	A = rot_l(A);
}

void CPU::ROL(NES::AddressingMode mode)
{
	uint16_t addr = param_addr(mode);
	write_to(addr, rot_l(read(addr)));
}

void CPU::RORA()
{
	A = rot_r(A);
}

void CPU::ROR(NES::AddressingMode mode)
{
	uint16_t addr = param_addr(mode);
	write_to(addr, rot_r(read(addr)));
}

// Load / store

void CPU::LD(uint8_t *reg, AddressingMode mode)
{
	uint8_t param = get_param(mode);
	*reg = param;
	P.Z = param == 0;
	P.N = param >> 7;
}

void CPU::ST(uint8_t reg, AddressingMode mode)
{
	write_to(param_addr(mode), reg);
}

// Stack

void CPU::T(uint8_t *reg_from, uint8_t *reg_to)
{
	*reg_to = *reg_from;
}

void CPU::PH(uint8_t value, bool set_b)
{
	if (set_b)
		P.B = 0x3;
	write_to(S, value);
	S--;
}

void CPU::PL(uint8_t *reg_to)
{
	uint8_t param = read(S);
	*reg_to = read(S);
	P.Z = param == 0;
	P.N = param >> 7;
	S++;
}
