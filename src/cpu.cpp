#include <2a03/cpu.h>
#include <iostream>
#include <cstring>

using namespace NES;

CPU::CPU(NES::MemoryBus &bus) : bus(bus) {}

void CPU::power()
{
	A = 0x0;
	X = 0x0;
	Y = 0x0;
	S = 0xFD;
	P.status = 0x24;
	
	IRQ = NMI = false;
	
	// TODO: Reenable once PPU is implemented
	while (false)
	{
		for (uint16_t i = 0x4000; i <= 0x4013; i++)
			bus.write(i, 0x0);
		bus.write(0x4015, 0x0); // All channels disabled
		bus.write(0x4017, 0x0); // Frame IRQ enabled
	}
	
	// TODO: Rest of power up logic
	// All 15 bits of noise channel LFSR = $0000[4]. The first time the LFSR
	// is clocked from the all-0s state, it will shift in a 1.
	
	reset();
}

void CPU::reset()
{
	interrupt(i_reset);
}

void CPU::execute()
{
	uint16_t initial_pc = PC;
	switch (bus.read(PC++))
	{
		case 0x10: BPL(); break;
		case 0x30: BMI(); break;
		case 0x50: BVC(); break;
		case 0x70: BVS(); break;
		case 0x90: BCC(); break;
		case 0xB0: BCS(); break;
		case 0xD0: BNE(); break;
		case 0xF0: BEQ(); break;
		case 0x4C: JMP(abs); break;
		case 0x6C: JMP(ind); break;
		case 0x20: JSR(); break;
		case 0x60: RTS(); break;
		case 0x40: RTI(); break;
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
		case 0x24: BIT(zp); break;
		case 0x2C: BIT(abs); break;
		case 0xC9: CMP(imm); break;
		case 0xC5: CMP(zp); break;
		case 0xD5: CMP(zp_x); break;
		case 0xCD: CMP(abs); break;
		case 0xDD: CMP(abs_x); break;
		case 0xD9: CMP(abs_y); break;
		case 0xC1: CMP(idx_ind_x); break;
		case 0xD1: CMP(ind_idx_y); break;
		case 0xE0: CP(X, imm); break;
		case 0xE4: CP(X, zp); break;
		case 0xEC: CP(X, abs); break;
		case 0xC0: CP(Y, imm); break;
		case 0xC4: CP(Y, zp); break;
		case 0xCC: CP(Y, abs); break;
		case 0xC6: DEC(zp); break;
		case 0xD6: DEC(zp_x); break;
		case 0xCE: DEC(abs); break;
		case 0xDE: DEC(abs_x); break;
		case 0x49: EOR(imm); break;
		case 0x45: EOR(zp); break;
		case 0x55: EOR(zp_x); break;
		case 0x4D: EOR(abs); break;
		case 0x5D: EOR(abs_x); break;
		case 0x59: EOR(abs_y); break;
		case 0x41: EOR(idx_ind_x); break;
		case 0x51: EOR(ind_idx_y); break;
		case 0x18: CLC(); break;
		case 0x38: SEC(); break;
		case 0x58: CLI(); break;
		case 0x78: SEI(); break;
		case 0xB8: CLV(); break;
		case 0xD8: CLD(); break;
		case 0xF8: SED(); break;
		case 0x4A: LSR_A(); break;
		case 0x46: LSR(zp); break;
		case 0x56: LSR(zp_x); break;
		case 0x4E: LSR(abs); break;
		case 0x5E: LSR(abs_x); break;
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
		case 0xE9: SBC(imm); break;
		case 0xE5: SBC(zp); break;
		case 0xF5: SBC(zp_x); break;
		case 0xED: SBC(abs); break;
		case 0xFD: SBC(abs_x); break;
		case 0xF9: SBC(abs_y); break;
		case 0xE1: SBC(idx_ind_x); break;
		case 0xF1: SBC(ind_idx_y); break;
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
		case 0xB6: LD(X, zp_y); break;
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
		case 0x96: ST(X, zp_y); break;
		case 0x8E: ST(X, abs); break;
		case 0x84: ST(Y, zp); break;
		case 0x94: ST(Y, zp_x); break;
		case 0x8C: ST(Y, abs); break;
		case 0xE6: INC(zp); break;
		case 0xF6: INC(zp_x); break;
		case 0xEE: INC(abs); break;
		case 0xFE: INC(abs_x); break;
		case 0xAA: T(A, X); break;
		case 0x8A: T(X, A); break;
		case 0xA8: T(A, Y); break;
		case 0x98: T(Y, A); break;
		case 0xCA: DE(X); break;
		case 0xE8: IN(X); break;
		case 0x88: DE(Y); break;
		case 0xC8: IN(Y); break;
		case 0x9A: T(X, S); break;
		case 0xBA: T(S, X); break;
		case 0x48: PH(A); break;
		case 0x08: PH(P); break;
		case 0x68: PL(A); break;
		case 0x28: PL(P); break;
		case 0xEA: /* NOP */ cycles += 2; break;
		// TODO: Implement BRK
		// Unofficial
		/* 1-byte NOPs */
		case 0x80: PC++; cycles += 2; break;
		case 0x04:
		case 0x44:
		case 0x64: PC++; cycles += 3; break;
		case 0x14:
		case 0x34:
		case 0x54:
		case 0x74:
		case 0xD4:
		case 0xF4: PC++; cycles += 4; break;
		/* 2-byte NOPs */
		case 0x0C: PC += 2; cycles += 4; break;
		case 0x1C:
		case 0x3C:
		case 0x5C:
		case 0x7C:
		case 0xDC:
		case 0xFC: PC += 2; cycles += 5; break;
		default:
			std::cerr << "Unhandled / invalid opcode: " << std::hex
				  << static_cast<int>(bus.read(initial_pc))
				  << std::endl;
			std::cerr.flush();
			throw InvalidOpcode();
	}
	if (NMI)
		interrupt(i_nmi);
	if (IRQ && !P.I)
		interrupt(i_irq);
}

void CPU::interrupt(NES::Interrupt type)
{
	if (type != i_reset)
	{
		PH((uint8_t)PC >> 8);
		PH((uint8_t)PC);
		PH(P.status);
	}
	else
	{
		P.status |= 0x04;
		// TODO: Reenable once the PPU is implemented
		while (false)
		{
			bus.write(0x4015, 0x0); // All channels disabled
		}
	}
	
	P.I = true;
	
	switch (type)
	{
		case i_nmi:
			PC = bus.read16(0xFFFA); break;
		case i_reset:
			PC = bus.read16(0xFFFC); break;
		case i_irq:
		case i_brk:
		default:
			PC = bus.read16(0xFFFE); break;
	}
	
	if (type == i_nmi)
		NMI = false;
	else if (type == i_irq)
		IRQ = false;
}

uint8_t CPU::get_operand(AddressingMode mode)
{
	return bus.read(operand_addr(mode));
}

bool CPU::is_same_page(uint16_t addr, uint16_t addr2)
{
	return (addr & 0xFF00) == (addr2 & 0xFF00);
}

uint16_t CPU::operand_addr(AddressingMode mode)
{
	uint16_t addr = 0x0;
	switch (mode)
	{
		case abs:
			addr = bus.read16(PC);
			PC += 2;
			break;
		case abs_x:
			addr = bus.read16(PC) + X;
			PC += 2;
			if (!is_same_page(addr - X, addr)) cycles++;
			break;
		case abs_y:
			addr = bus.read16(PC) + Y;
			PC += 2;
			if (!is_same_page(addr - Y, addr)) cycles++;
			break;
		case imm:
			addr = PC;
			PC++;
			break;
		case zp:
			addr = bus.read(PC);
			PC++;
			break;
		case zp_x:
			addr = (bus.read(PC) + X) % 0x100;
			PC++;
			break;
		case zp_y:
			addr = (bus.read(PC) + Y) % 0x100;
			PC++;
			break;
		case idx_ind_x:
			addr = bus.read16((bus.read(PC) + X) % 0x100, true);
			PC++;
			break;
		case ind_idx_y:
			addr = bus.read16(bus.read(PC), true) + Y;
			PC++;
			if (!is_same_page(addr - Y, addr)) cycles++;
			break;
		case ind:
		default:
			std::cerr << "Invalid addressing mode: "
				<< std::hex << int(mode) << std::endl;
			std::cerr.flush();
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

void CPU::do_ADC(uint8_t operand)
{
	// ADC/SBC implementation:
	// https://stackoverflow.com/questions/29193303/6502-emulation-proper-way-to-implement-adc-and-sbc
	// Overflow on signed arithmetic:
	// http://www.6502.org/tutorials/vflag.html
	if (P.D)
	{
		// In decimal mode treat operands as binary-coded decimals.
		// Decimal mode is not available on 2A03. Since this is a NES
		// emulator, we don't support decimal mode, which is available
		// in a 6502.
		std::cerr << "ADC/SBC op with decimal mode is unavailable."
			  << std::endl;
		std::cerr.flush();
		return;
	}
	
	uint16_t sum = A + operand + P.C;
	
	P.C = sum > 0xFF;
	
	// Two-complements overflow happens when the result value is outside of
	// the [-128, 127] range. This is equivalent to when two operands with
	// the same sign are added and the result has a different sign.
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

void CPU::set_NZ(uint8_t value)
{
	P.Z = value == 0;
	P.N = value >> 7;
}

// Branch instructions

void CPU::branch_rel()
{
	uint8_t op = get_operand(imm);
	cycles++;
	if ((PC & 0xFF00) != ((PC + op) & 0xFF00))
		cycles++;
	PC += op;
}

#define branch_rel_if(expr) \
{ \
	cycles += 2; \
	if (expr) branch_rel(); \
	else PC++; \
}

void CPU::BPL() branch_rel_if(!P.N)
void CPU::BMI() branch_rel_if(P.N)
void CPU::BVC() branch_rel_if(!P.V)
void CPU::BVS() branch_rel_if(P.V)
void CPU::BCC() branch_rel_if(!P.C)
void CPU::BCS() branch_rel_if(P.C)
void CPU::BNE() branch_rel_if(!P.Z)
void CPU::BEQ() branch_rel_if(P.Z)

// Control transfer

void CPU::JMP(AddressingMode mode)
{
	switch (mode)
	{
		case abs:
			PC = bus.read16(PC);
			cycles += 3;
			break;
		case ind:
			// Vector beginning on a last byte of a page will take
			// the high byte of the address from the beginning of
			// the same page rather than the next one.
			// $3000 = $40
			// $30FF = $80
			// $3100 = $50
			// JMP ($30FF) will transfer control to $4080 rather
			// than $5080.
			uint16_t h_addr, l_addr;
			l_addr = bus.read16(PC);
			h_addr = (l_addr % 0x100 == 0xFF)
				? (uint16_t)(l_addr - l_addr % 0x100)
				: (uint16_t)(l_addr + 1);
			PC = (uint16_t)(bus.read(h_addr)) << 8 | bus.read(l_addr);
			cycles += 5;
			break;
		default:
			std::cerr << "Invalid addressing mode for JMP: " << mode
				<< std::endl;
			std::cerr.flush();
	}
}

void CPU::JSR()
{
	// JSR return address should be the last byte of the 3-byte JSR instr.
	auto return_addr = (uint16_t)(PC + 1);
	PH((uint8_t)(return_addr >> 8));
	PH((uint8_t)return_addr);
	JMP(abs);
	cycles += 6;
}

void CPU::RTS()
{
	uint8_t l_addr, h_addr;
	PL(l_addr);
	PL(h_addr);
	PC = (h_addr << 8 | l_addr) + (uint8_t)0x1;
	cycles += 6;
}

void CPU::RTI()
{
	uint8_t l_addr, h_addr;
	PL(P.status);
	PL(l_addr);
	PL(h_addr);
	PC = (h_addr << 8 | l_addr);
	cycles += 6;
}

// Arithmetic / logical

void CPU::ADC(AddressingMode mode)
{
	uint16_t op_addr = operand_addr(mode);
	do_ADC(bus.read(op_addr));
	switch (mode)
	{
		case imm: 	cycles += 2; break;
		case zp: 	cycles += 3; break;
		case zp_x: 	cycles += 4; break;
		case abs: 	cycles += 4; break;
		case abs_x: 	cycles += 4; break;
		case abs_y: 	cycles += 4; break;
		case idx_ind_x: cycles += 6; break;
		case ind_idx_y: cycles += 5; break;
		default:	std::cerr << "Invalid addressing mode for ADC."
				<< std::endl;
	}
}

void CPU::AND(AddressingMode mode)
{
	A &= get_operand(mode);
	set_NZ(A);
	switch (mode)
	{
		case imm: 	cycles += 2; break;
		case zp: 	cycles += 3; break;
		case zp_x: 	cycles += 4; break;
		case abs: 	cycles += 4; break;
		case abs_x: 	cycles += 4; break;
		case abs_y: 	cycles += 4; break;
		case idx_ind_x: cycles += 6; break;
		case ind_idx_y: cycles += 5; break;
		default:	std::cerr << "Invalid addressing mode for AND."
				<< std::endl;
	}
}

void CPU::ASL_A()
{
	A = shift_l(A);
	cycles += 2;
}

void CPU::ASL(AddressingMode mode)
{
	uint16_t addr = operand_addr(mode);
	bus.write(addr, shift_l(bus.read(addr)));
	switch (mode)
	{
		case zp: 	cycles += 5; break;
		case zp_x: 	cycles += 6; break;
		case abs: 	cycles += 6; break;
		case abs_x: 	cycles += 7; break;
		default: 	std::cerr << "Invalid addressing mode for ASL."
				<< std::endl;
	}
}

void CPU::BIT(AddressingMode mode)
{
	uint8_t operand = get_operand(mode);
	P.Z = (A & operand) == 0;
	P.V = (bool)(operand >> 6);
	P.N = (bool)(operand >> 7);
	if (mode == zp)
		cycles += 3;
	else if (mode == abs)
		cycles += 4;
}

void CPU::CMP(AddressingMode mode)
{
	CP(A, mode);
}

void CPU::CP(uint8_t &reg, AddressingMode mode)
{
	uint8_t operand = get_operand(mode);
	P.Z = reg == operand;
	P.C = reg >= operand;
	P.N = (reg - operand) >> 7;
	switch (mode)
	{
		case imm: 	cycles += 2; break;
		case zp: 	cycles += 3; break;
		case zp_x: 	cycles += 4; break;
		case abs: 	cycles += 4; break;
		case abs_x: 	cycles += 4; break;
		case abs_y: 	cycles += 4; break;
		case idx_ind_x: cycles += 6; break;
		case ind_idx_y: cycles += 5; break;
		default:	std::cerr << "Invalid addressing mode for CPx."
				<< std::endl;
	}
}

void CPU::DEC(AddressingMode mode)
{
	// TODO: Is below being uint8_t instead of uint16_t a bug?
	uint8_t op_addr = operand_addr(mode);
	uint8_t result = bus.read(op_addr) - 1;
	bus.write(op_addr, result);
	set_NZ(result);
	switch (mode)
	{
		case zp: 	cycles += 5; break;
		case zp_x: 	cycles += 6; break;
		case abs: 	cycles += 6; break;
		case abs_x: 	cycles += 7; break;
		default:	std::cerr << "Invalid addressing mode for DEC."
				<< std::endl;
	}
}

void CPU::EOR(AddressingMode mode)
{
	uint8_t operand = get_operand(mode);
	A ^= operand;
	set_NZ(A);
	switch (mode)
	{
		case imm: 	cycles += 2; break;
		case zp: 	cycles += 3; break;
		case zp_x: 	cycles += 4; break;
		case abs: 	cycles += 4; break;
		case abs_x: 	cycles += 4; break;
		case abs_y: 	cycles += 4; break;
		case idx_ind_x: cycles += 6; break;
		case ind_idx_y: cycles += 5; break;
		default:	std::cerr << "Invalid addressing mode for EOR."
				<< std::endl;
	}
}

#define set_status_flag(flag, value) \
{ \
	P.flag = value; \
	cycles += 2; \
} 

void CPU::CLC() set_status_flag(C, false)
void CPU::SEC() set_status_flag(C, true)
void CPU::CLI() set_status_flag(I, false)
void CPU::SEI() set_status_flag(I, true)
void CPU::CLV() set_status_flag(V, false)
void CPU::CLD() set_status_flag(D, false)
void CPU::SED() set_status_flag(D, true)

void CPU::LSR_A()
{
	A = shift_r(A);
}

void CPU::LSR(AddressingMode mode)
{
	uint16_t addr = operand_addr(mode);
	bus.write(addr, shift_r(bus.read(addr)));
}


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
	bus.write(addr, rot_l(bus.read(addr)));
}

void CPU::ROR_A()
{
	A = rot_r(A);
}

void CPU::ROR(AddressingMode mode)
{
	uint16_t addr = operand_addr(mode);
	bus.write(addr, rot_r(bus.read(addr)));
}

void CPU::SBC(AddressingMode mode)
{
	uint16_t op_addr = operand_addr(mode);
	do_ADC(~bus.read(op_addr));
	switch (mode)
	{
		case imm: cycles += 2; break;
		case zp: cycles += 3; break;
		case zp_x: cycles += 4; break;
		case abs: cycles += 4; break;
		case abs_x: cycles += 4; break;
		case abs_y: cycles += 4; break;
		case idx_ind_x: cycles += 6; break;
		case ind_idx_y: cycles += 5; break;
		default:
			std::cerr << "Invalid addressing mode for ADC."
				  << std::endl;
	}
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
	bus.write(operand_addr(mode), reg);
}

void CPU::INC(AddressingMode mode)
{
	uint16_t addr = operand_addr(mode);
	auto newval = (uint8_t)(bus.read(addr) + 1);
	bus.write(addr, newval);
	set_NZ(newval);
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
	bus.write((uint16_t)(0x100 + S), value);
	S--;
}

void CPU::PH(StatusRegister &p)
{
	PH(p.status);
}

void CPU::PL(uint8_t &reg_to)
{
	S++;
	uint8_t operand = bus.read((uint16_t)(0x100 + S));
	reg_to = operand;
	set_NZ(operand);
}

void CPU::PL(StatusRegister &p)
{
	PL(p.status);
}