#include <2a03/cpu.h>
#include <iostream>
#include <cstring>

using namespace NES;

void CPU::power()
{
	A = 0x0;
	X = 0x0;
	Y = 0x0;
	S = 0xFD;
	P.status = 0x24;
	
	// TODO: Rest of power up logic
	// $4017 = $00 (frame irq enabled)
	// $4015 = $00 (all channels disabled)
	// $4000-$400F = $00 (not sure about $4010-$4013)
	// All 15 bits of noise channel LFSR = $0000[4]. The first time the LFSR
	// is clocked from the all-0s state, it will shift in a 1.
	
	// RAM state is not consistent on power up on a real machine, but we'll
	// clear it here.
	memset(ram, 0xFF, sizeof(ram));
}

void CPU::reset()
{
	S -= 3;
	P.status |= 0x04;
	
	// TODO: Rest of reset logic
	// APU was silenced ($4015 = 0)
	// APU triangle phase is reset to 0
}

void CPU::execute()
{
	switch (read(PC++))
	{
		// Control transfer
		case 0x4C: JMP(abs); break;
		case 0x6C: JMP(ind); break;
		case 0x20: JSR(); break;
		case 0x60: RTS(); break;
		// Arithmetic / logical
		case 0x09: ORA(imm); break;
		case 0x05: ORA(zp); break;
		case 0x15: ORA(zp_x); break;
		case 0x0D: ORA(abs); break;
		case 0x1D: ORA(abs_x); break;
		case 0x19: ORA(abs_y); break;
		case 0x01: ORA(idx_ind_x); break;
		case 0x11: ORA(ind_idx_y); break;
		case 0x2A: ROL_A(); break;
		case 0x26: ROL(zp); break;
		case 0x36: ROL(zp_x); break;
		case 0x2E: ROL(abs); break;
		case 0x3E: ROL(abs_x); break;
		case 0x6A: ROR_A(); break;
		case 0x66: ROR(zp); break;
		case 0x76: ROR(zp_x); break;
		case 0x6E: ROR(abs); break;
		case 0x7E: ROR(abs_x); break;
		case 0x69: ADC(imm); break;
		case 0x65: ADC(zp); break;
		case 0x75: ADC(zp_x); break;
		case 0x6D: ADC(abs); break;
		case 0x7D: ADC(abs_x); break;
		case 0x79: ADC(abs_y); break;
		case 0x61: ADC(idx_ind_x); break;
		case 0x71: ADC(ind_idx_y); break;
		case 0x29: AND(imm); break;
		case 0x25: AND(zp); break;
		case 0x35: AND(zp_x); break;
		case 0x2D: AND(abs); break;
		case 0x3D: AND(abs_x); break;
		case 0x39: AND(abs_y); break;
		case 0x21: AND(idx_ind_x); break;
		case 0x31: AND(ind_idx_y); break;
		case 0x0A: ASL_A(); break;
		case 0x06: ASL(zp); break;
		case 0x16: ASL(zp_x); break;
		case 0x0E: ASL(abs); break;
		case 0x1E: ASL(abs_x); break;
		case 0x4A: LSR_A(); break;
		case 0x46: LSR(zp); break;
		case 0x56: LSR(zp_x); break;
		case 0x4E: LSR(abs); break;
		case 0x5E: LSR(abs_x); break;
		case 0xE9: SBC(imm); break;
		case 0xE5: SBC(zp); break;
		case 0xF5: SBC(zp_x); break;
		case 0xED: SBC(abs); break;
		case 0xFD: SBC(abs_x); break;
		case 0xF9: SBC(abs_y); break;
		case 0xE1: SBC(idx_ind_x); break;
		case 0xF1: SBC(ind_idx_y); break;
		// Load / store
		case 0xA9: LD(A, imm); break;
		case 0xA5: LD(A, zp); break;
		case 0xB5: LD(A, zp_x); break;
		case 0xAD: LD(A, abs); break;
		case 0xBD: LD(A, abs_x); break;
		case 0xB9: LD(A, abs_y); break;
		case 0xA1: LD(A, idx_ind_x); break;
		case 0xB1: LD(A, ind_idx_y); break;
		case 0xA2: LD(X, imm); break;
		case 0xA6: LD(X, zp); break;
		case 0xB6: LD(X, mode_zp_y); break;
		case 0xAE: LD(X, abs); break;
		case 0xBE: LD(X, abs_y); break;
		case 0xA0: LD(Y, imm); break;
		case 0xA4: LD(Y, zp); break;
		case 0xB4: LD(Y, zp_x); break;
		case 0xAC: LD(Y, abs); break;
		case 0xBC: LD(Y, abs_x); break;
		case 0x85: ST(A, zp); break;
		case 0x95: ST(A, zp_x); break;
		case 0x8D: ST(A, abs); break;
		case 0x9D: ST(A, abs_x); break;
		case 0x99: ST(A, abs_y); break;
		case 0x81: ST(A, idx_ind_x); break;
		case 0x91: ST(A, ind_idx_y); break;
		case 0x86: ST(X, zp); break;
		case 0x96: ST(X, mode_zp_y); break;
		case 0x8E: ST(X, abs); break;
		case 0x84: ST(Y, zp); break;
		case 0x94: ST(Y, zp_x); break;
		case 0x8C: ST(Y, abs); break;
		// Register
		case 0xAA: T(A, X); break;
		case 0x8A: T(X, A); break;
		case 0xA8: T(A, Y); break;
		case 0x98: T(Y, A); break;
		case 0xCA: DE(X); break;
		case 0xE8: IN(X); break;
		case 0x88: DE(Y); break;
		case 0xC8: IN(Y); break;
		// Stack
		case 0x9A: T(X, S); break;
		case 0xBA: T(S, X); break;
		case 0x48: PH(A); break;
		case 0x08: PH(P); break;
		case 0x68: PL(A); break;
		case 0x28: PL(P); break;
		// Others
		case 0xEA: /* NOP */ break;
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
			// 0x0000 - 0x00FF is zero page
			// 0x0100 - 0x01FF is stack memory
			// 0x0200 - 0x07FF is RAM
			ram[addr % 0x800] = val;
		default:
			std::cerr << "Unhandled write to " << std::hex << addr
				<< " with value: " << val << std::endl;
	}
}

uint8_t CPU::get_operand(AddressingMode mode)
{
	return read(operand_addr(mode));
}

uint16_t CPU::operand_addr(AddressingMode mode)
{
	uint16_t addr = 0x0;
	switch (mode)
	{
		case abs: 	addr = read16(PC); PC += 2; break;
		case abs_x: 	addr = read16(PC) + X; PC += 2; break;
		case abs_y: 	addr = read16(PC) + Y; PC += 2; break;
		case imm: 	addr = PC; PC++; break;
		case zp: 	addr = read(PC); PC++; break;
		case zp_x: 	addr = (read(PC) + X) % 0x100; PC++; break;
		case mode_zp_y: addr = (read(PC) + Y) % 0x100; PC++; break;
		case idx_ind_x: addr = read16((read(PC) + X) % 0x100, true);
				PC++; break;
		case ind_idx_y: addr = read16(read(PC), true) + Y; PC++; break;
		case ind:
		default:	std::cerr << "Invalid addressing mode: " << mode
				<< std::endl;
	}
	return addr;
}

// Auxiliary

uint8_t CPU::rot_l(uint8_t value)
{
	bool last_C = P.C;
	P.C = value >> (sizeof(value) * 8 - 1);
	uint8_t output = value << 1 | last_C;
	set_NZ(output);
	return output;
}

uint8_t CPU::rot_r(uint8_t value)
{
	bool last_C = P.C;
	P.C = value << (sizeof(value) * 8 - 1);
	uint8_t output = value >> 1 | last_C;
	set_NZ(output);
	return output;
}

void CPU::set_NZ(uint8_t value)
{
	P.Z = value == 0;
	P.N = value >> 7;
}

// Control transfer

void CPU::JMP(AddressingMode mode)
{
	switch (mode)
	{
		case ind:
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
		case abs:
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
	JMP(abs);
}

void CPU::RTS()
{
	uint8_t l_addr, h_addr;
	PL(l_addr);
	PL(h_addr);
	PC = h_addr << 8 | l_addr;
}

// Arithmetic / logical

void CPU::ORA(AddressingMode mode)
{
	uint8_t operand = get_operand(mode);
	A |= operand;
	set_NZ(A);
}

void CPU::ROL_A()
{
	A = rot_l(A);
}

void CPU::ROL(AddressingMode mode)
{
	uint16_t addr = operand_addr(mode);
	write_to(addr, rot_l(read(addr)));
}

void CPU::ROR_A()
{
	A = rot_r(A);
}

void CPU::ROR(AddressingMode mode)
{
	uint16_t addr = operand_addr(mode);
	write_to(addr, rot_r(read(addr)));
}

// ADC/SBC implementation:
// https://stackoverflow.com/questions/29193303/6502-emulation-proper-way-to-implement-adc-and-sbc
// Overflow on signed arithmetic:
// http://www.6502.org/tutorials/vflag.html

void CPU::ADC(AddressingMode mode)
{
	do_ADC(get_operand(mode));
}

void CPU::SBC(AddressingMode mode)
{
	do_ADC(~get_operand(mode));
}

void CPU::do_ADC(uint8_t operand)
{
	if (P.D)
	{
		// In decimal mode treat operands as binary-coded decimals.
		// Decimal mode is not available on 2A03. Since this is a NES
		// emulator, we don't support decimal mode, which is actually
		// available in a MOS 6502.
		std::cout << "ADC/SBC op with decimal mode is unavailable."
			  << std::endl;
		return;
	}
	
	uint16_t sum = A + operand + P.C;
	
	P.C = sum > 0xFF;
	
	// Two-complements overflow happens when the result value is outside of
	// the [-128, 127] range. According to post above two numbers this is
	// equivalent to when two operands with the same sign are added and the
	// result has a different sign.
	// ~(A ^ operand) 	7th bit is 1 if A and operand have the same sign.
	// A ^ sum 		7th bit is 1 if A | operand sign differs from sum.
	// 0x80 		We're interested in the 7th bit only.
	// >> 7			Shift the most significant bit to least
	//			significant position and cast to bool so we
	//			we don't have a type-related warning.
	P.V = (bool)((~(A ^ operand) & (A ^ sum) & 0x80) >> 7);
	
	A = (uint8_t)sum;
	
	set_NZ(A);
}

uint8_t CPU::shift_l(uint8_t value)
{
	bool last_C = P.C;
	P.C = value >> (sizeof(value) * 8 - 1);
	uint8_t output = (uint8_t)(value << 1);
	set_NZ(output);
	return output;
}

uint8_t CPU::shift_r(uint8_t value)
{
	bool last_C = P.C;
	P.C = value << (sizeof(value) * 8 - 1);
	uint8_t output = (uint8_t)(value >> 1);
	set_NZ(output);
	return output;
}

void CPU::AND(AddressingMode mode)
{
	A &= get_operand(mode);
	set_NZ(A);
}

void CPU::ASL_A()
{
	A = shift_l(A);
}

void CPU::ASL(AddressingMode mode)
{
	uint16_t addr = operand_addr(mode);
	write_to(addr, shift_l(read(addr)));
}

void CPU::LSR_A()
{
	A = shift_r(A);
}

void CPU::LSR(AddressingMode mode)
{
	uint16_t addr = operand_addr(mode);
	write_to(addr, shift_r(read(addr)));
}

// Load / store

void CPU::LD(uint8_t &reg, AddressingMode mode)
{
	uint8_t operand = get_operand(mode);
	reg = operand;
	set_NZ(operand);
}

void CPU::ST(uint8_t reg, AddressingMode mode)
{
	write_to(operand_addr(mode), reg);
}

// Register

void CPU::T(uint8_t &reg_from, uint8_t &reg_to)
{
	reg_to = reg_from;
	set_NZ(reg_from);
}

void CPU::DE(uint8_t &reg)
{
	reg--;
	set_NZ(reg);
}

void CPU::IN(uint8_t &reg)
{
	reg++;
	set_NZ(reg);
}

// Stack

void CPU::PH(uint8_t value)
{
	write_to(S, value);
	S--;
}

void CPU::PH(StatusRegister &p)
{
	PH(p.status);
	p.B = 0x3;
}

void CPU::PL(uint8_t &reg_to)
{
	uint8_t operand = read(S);
	S++;
	reg_to = operand;
	set_NZ(operand);
}

void CPU::PL(StatusRegister &p)
{
	PL(p.status);
}