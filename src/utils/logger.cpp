#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <2a03/utils/logger.h>

NES::CPULogger::CPULogger(NES::CPU &cpu, NES::MemoryBus &bus) :
	cpu(cpu),
	bus(bus),
	logs(),
	is_cout_each_line_enabled(true)
{}

void NES::CPULogger::log()
{
	std::string line;
	std::stringstream ss;
	int opcode;
	
	// PC as a 4-char wide hex string.
	ss << std::setfill('0') << std::setw(4) << std::hex << (int)cpu.PC;
	line += std::string(ss.str());
	ss.str(std::string());
	
	line += "  ";
	
	// Current opcode as hex.
	opcode = (int)bus.read(cpu.PC);
	ss << std::setfill('0') << std::setw(2) << std::hex << opcode;
	line += std::string(ss.str());
	ss.str(std::string());
	
	line += " ";
	
	// TODO: Fill opcode parameters here instead of empty space.
	line += "       ";
	
	// Decode opcode to string.
	line += decode(opcode);
	
	// Convert to uppercase chars and push to log storage.
	std::transform(line.begin(), line.end(), line.begin(),
		[](char c) -> char { return (char)std::toupper(c); });
	
	logs.push_back(line);
	
	if (is_cout_each_line_enabled)
		std::cout << line << std::endl;
}

std::string NES::CPULogger::decode(uint8_t opcode)
{
	switch (opcode) {
//		case 0x10: BPL;
//		case 0x30: BMI;
//		case 0x50: BVC;
//		case 0x70: BVS;
//		case 0x90: BCC;
//		case 0xB0: BCS;
//		case 0xD0: BNE;
//		case 0xF0: BEQ;
//		case 0x4C: JMP(abs);
//		case 0x6C: JMP(ind);
//		case 0x20: JSR;
//		case 0x60: RTS;
//		case 0x40: RTI;
//		case 0x69: ADC(imm); break;
//		case 0x65: ADC(zp); break;
//		case 0x75: ADC(zp_x); break;
//		case 0x6D: ADC(abs); break;
//		case 0x7D: ADC(abs_x); break;
//		case 0x79: ADC(abs_y); break;
//		case 0x61: ADC(idx_ind_x); break;
//		case 0x71: ADC(ind_idx_y); break;
//		case 0x29: AND(imm); break;
//		case 0x25: AND(zp); break;
//		case 0x35: AND(zp_x); break;
//		case 0x2D: AND(abs); break;
//		case 0x3D: AND(abs_x); break;
//		case 0x39: AND(abs_y); break;
//		case 0x21: AND(idx_ind_x); break;
//		case 0x31: AND(ind_idx_y); break;
//		case 0x0A: ASL_A(); break;
//		case 0x06: ASL(zp); break;
//		case 0x16: ASL(zp_x); break;
//		case 0x0E: ASL(abs); break;
//		case 0x1E: ASL(abs_x); break;
//		case 0x24: BIT(zp); break;
//		case 0x2C: BIT(abs); break;
//		case 0xC9: CMP(imm); break;
//		case 0xC5: CMP(zp); break;
//		case 0xD5: CMP(zp_x); break;
//		case 0xCD: CMP(abs); break;
//		case 0xDD: CMP(abs_x); break;
//		case 0xD9: CMP(abs_y); break;
//		case 0xC1: CMP(idx_ind_x); break;
//		case 0xD1: CMP(ind_idx_y); break;
//		case 0xE0: CP(X, imm); break;
//		case 0xE4: CP(X, zp); break;
//		case 0xEC: CP(X, abs); break;
//		case 0xC0: CP(Y, imm); break;
//		case 0xC4: CP(Y, zp); break;
//		case 0xCC: CP(Y, abs); break;
//		case 0xC6: DEC(zp); break;
//		case 0xD6: DEC(zp_x); break;
//		case 0xCE: DEC(abs); break;
//		case 0xDE: DEC(abs_x); break;
//		case 0x49: EOR(imm); break;
//		case 0x45: EOR(zp); break;
//		case 0x55: EOR(zp_x); break;
//		case 0x4D: EOR(abs); break;
//		case 0x5D: EOR(abs_x); break;
//		case 0x59: EOR(abs_y); break;
//		case 0x41: EOR(idx_ind_x); break;
//		case 0x51: EOR(ind_idx_y); break;
//		case 0x18: CLC(); break;
//		case 0x38: SEC(); break;
//		case 0x58: CLI(); break;
//		case 0x78: SEI(); break;
//		case 0xB8: CLV(); break;
//		case 0xD8: CLD(); break;
//		case 0xF8: SED(); break;
//		case 0x4A: LSR_A(); break;
//		case 0x46: LSR(zp); break;
//		case 0x56: LSR(zp_x); break;
//		case 0x4E: LSR(abs); break;
//		case 0x5E: LSR(abs_x); break;
//		case 0x09: ORA(imm); break;
//		case 0x05: ORA(zp); break;
//		case 0x15: ORA(zp_x); break;
//		case 0x0D: ORA(abs); break;
//		case 0x1D: ORA(abs_x); break;
//		case 0x19: ORA(abs_y); break;
//		case 0x01: ORA(idx_ind_x); break;
//		case 0x11: ORA(ind_idx_y); break;
//		case 0x2A: ROL_A(); break;
//		case 0x26: ROL(zp); break;
//		case 0x36: ROL(zp_x); break;
//		case 0x2E: ROL(abs); break;
//		case 0x3E: ROL(abs_x); break;
//		case 0x6A: ROR_A(); break;
//		case 0x66: ROR(zp); break;
//		case 0x76: ROR(zp_x); break;
//		case 0x6E: ROR(abs); break;
//		case 0x7E: ROR(abs_x); break;
//		case 0xE9: SBC(imm); break;
//		case 0xE5: SBC(zp); break;
//		case 0xF5: SBC(zp_x); break;
//		case 0xED: SBC(abs); break;
//		case 0xFD: SBC(abs_x); break;
//		case 0xF9: SBC(abs_y); break;
//		case 0xE1: SBC(idx_ind_x); break;
//		case 0xF1: SBC(ind_idx_y); break;
//		case 0xA9: LD(A, imm); break;
//		case 0xA5: LD(A, zp); break;
//		case 0xB5: LD(A, zp_x); break;
//		case 0xAD: LD(A, abs); break;
//		case 0xBD: LD(A, abs_x); break;
//		case 0xB9: LD(A, abs_y); break;
//		case 0xA1: LD(A, idx_ind_x); break;
//		case 0xB1: LD(A, ind_idx_y); break;
//		case 0xA2: LD(X, imm); break;
//		case 0xA6: LD(X, zp); break;
//		case 0xB6: LD(X, mode_zp_y); break;
//		case 0xAE: LD(X, abs); break;
//		case 0xBE: LD(X, abs_y); break;
//		case 0xA0: LD(Y, imm); break;
//		case 0xA4: LD(Y, zp); break;
//		case 0xB4: LD(Y, zp_x); break;
//		case 0xAC: LD(Y, abs); break;
//		case 0xBC: LD(Y, abs_x); break;
//		case 0x85: ST(A, zp); break;
//		case 0x95: ST(A, zp_x); break;
//		case 0x8D: ST(A, abs); break;
//		case 0x9D: ST(A, abs_x); break;
//		case 0x99: ST(A, abs_y); break;
//		case 0x81: ST(A, idx_ind_x); break;
//		case 0x91: ST(A, ind_idx_y); break;
//		case 0x86: ST(X, zp); break;
//		case 0x96: ST(X, mode_zp_y); break;
//		case 0x8E: ST(X, abs); break;
//		case 0x84: ST(Y, zp); break;
//		case 0x94: ST(Y, zp_x); break;
//		case 0x8C: ST(Y, abs); break;
//		case 0xAA: T(A, X); break;
//		case 0x8A: T(X, A); break;
//		case 0xA8: T(A, Y); break;
//		case 0x98: T(Y, A); break;
//		case 0xCA: DE(X); break;
//		case 0xE8: IN(X); break;
//		case 0x88: DE(Y); break;
//		case 0xC8: IN(Y); break;
//		case 0x9A: T(X, S); break;
//		case 0xBA: T(S, X); break;
//		case 0x48: PH(A); break;
//		case 0x08: PH(P); break;
//		case 0x68: PL(A); break;
//		case 0x28: PL(P); break;
//		case 0xEA: NOP; break;
		default: return "???";
	}
}