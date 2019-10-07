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
		case 0x10: return "BPL";
		case 0x30: return "BMI";
		case 0x50: return "BVC";
		case 0x70: return "BVS";
		case 0x90: return "BCC";
		case 0xB0: return "BCS";
		case 0xD0: return "BNE";
		case 0xF0: return "BEQ";
		case 0x4C: return "JMP"; //(abs);
		case 0x6C: return "JMP"; //(ind);
		case 0x20: return "JSR";
		case 0x60: return "RTS";
		case 0x40: return "RTI";
		case 0x69: return "ADC"; //(imm); break;
		case 0x65: return "ADC"; //(zp); break;
		case 0x75: return "ADC"; //(zp_x); break;
		case 0x6D: return "ADC"; //(abs); break;
		case 0x7D: return "ADC"; //(abs_x); break;
		case 0x79: return "ADC"; //(abs_y); break;
		case 0x61: return "ADC"; //(idx_ind_x); break;
		case 0x71: return "ADC"; //(ind_idx_y); break;
		case 0x29: return "AND"; //(imm); break;
		case 0x25: return "AND"; //(zp); break;
		case 0x35: return "AND"; //(zp_x); break;
		case 0x2D: return "AND"; //(abs); break;
		case 0x3D: return "AND"; //(abs_x); break;
		case 0x39: return "AND"; //(abs_y); break;
		case 0x21: return "AND"; //(idx_ind_x); break;
		case 0x31: return "AND"; //(ind_idx_y); break;
		case 0x0A: return "ASL A"; // break;
		case 0x06: return "ASL"; //(zp); break;
		case 0x16: return "ASL"; //(zp_x); break;
		case 0x0E: return "ASL"; //(abs); break;
		case 0x1E: return "ASL"; //(abs_x); break;
		case 0x24: return "BIT"; //(zp); break;
		case 0x2C: return "BIT"; //(abs); break;
		case 0xC9: return "CMP"; //(imm); break;
		case 0xC5: return "CMP"; //(zp); break;
		case 0xD5: return "CMP"; //(zp_x); break;
		case 0xCD: return "CMP"; //(abs); break;
		case 0xDD: return "CMP"; //(abs_x); break;
		case 0xD9: return "CMP"; //(abs_y); break;
		case 0xC1: return "CMP"; //(idx_ind_x); break;
		case 0xD1: return "CMP"; //(ind_idx_y); break;
		case 0xE0: return "CPX"; // imm); break;
		case 0xE4: return "CPX"; // zp); break;
		case 0xEC: return "CPX"; // abs); break;
		case 0xC0: return "CPY"; // imm); break;
		case 0xC4: return "CPY"; // zp); break;
		case 0xCC: return "CPY"; // abs); break;
		case 0xC6: return "DEC"; //(zp); break;
		case 0xD6: return "DEC"; //(zp_x); break;
		case 0xCE: return "DEC"; //(abs); break;
		case 0xDE: return "DEC"; //(abs_x); break;
		case 0x49: return "EOR"; //(imm); break;
		case 0x45: return "EOR"; //(zp); break;
		case 0x55: return "EOR"; //(zp_x); break;
		case 0x4D: return "EOR"; //(abs); break;
		case 0x5D: return "EOR"; //(abs_x); break;
		case 0x59: return "EOR"; //(abs_y); break;
		case 0x41: return "EOR"; //(idx_ind_x); break;
		case 0x51: return "EOR"; //(ind_idx_y); break;
		case 0x18: return "CLC"; //(); break;
		case 0x38: return "SEC"; //(); break;
		case 0x58: return "CLI"; //(); break;
		case 0x78: return "SEI"; //(); break;
		case 0xB8: return "CLV"; //(); break;
		case 0xD8: return "CLD"; //(); break;
		case 0xF8: return "SED"; //(); break;
		case 0x4A: return "LSR_A"; //(); break;
		case 0x46: return "LSR"; //(zp); break;
		case 0x56: return "LSR"; //(zp_x); break;
		case 0x4E: return "LSR"; //(abs); break;
		case 0x5E: return "LSR"; //(abs_x); break;
		case 0x09: return "ORA"; //(imm); break;
		case 0x05: return "ORA"; //(zp); break;
		case 0x15: return "ORA"; //(zp_x); break;
		case 0x0D: return "ORA"; //(abs); break;
		case 0x1D: return "ORA"; //(abs_x); break;
		case 0x19: return "ORA"; //(abs_y); break;
		case 0x01: return "ORA"; //(idx_ind_x); break;
		case 0x11: return "ORA"; //(ind_idx_y); break;
		case 0x2A: return "ROL_A"; //(); break;
		case 0x26: return "ROL"; //(zp); break;
		case 0x36: return "ROL"; //(zp_x); break;
		case 0x2E: return "ROL"; //(abs); break;
		case 0x3E: return "ROL"; //(abs_x); break;
		case 0x6A: return "ROR_A"; //(); break;
		case 0x66: return "ROR"; //(zp); break;
		case 0x76: return "ROR"; //(zp_x); break;
		case 0x6E: return "ROR"; //(abs); break;
		case 0x7E: return "ROR"; //(abs_x); break;
		case 0xE9: return "SBC"; //(imm); break;
		case 0xE5: return "SBC"; //(zp); break;
		case 0xF5: return "SBC"; //(zp_x); break;
		case 0xED: return "SBC"; //(abs); break;
		case 0xFD: return "SBC"; //(abs_x); break;
		case 0xF9: return "SBC"; //(abs_y); break;
		case 0xE1: return "SBC"; //(idx_ind_x); break;
		case 0xF1: return "SBC"; //(ind_idx_y); break;
		case 0xA9: return "LDA"; //(imm); break;
		case 0xA5: return "LDA"; //(zp); break;
		case 0xB5: return "LDA"; //(zp_x); break;
		case 0xAD: return "LDA"; //(abs); break;
		case 0xBD: return "LDA"; //(abs_x); break;
		case 0xB9: return "LDA"; //(abs_y); break;
		case 0xA1: return "LDA"; //(idx_ind_x); break;
		case 0xB1: return "LDA"; //(ind_idx_y); break;
		case 0xA2: return "LDX"; //(imm); break;
		case 0xA6: return "LDX"; //(zp); break;
		case 0xB6: return "LDX"; //(mode_zp_y); break;
		case 0xAE: return "LDX"; //(abs); break;
		case 0xBE: return "LDX"; //(abs_y); break;
		case 0xA0: return "LDY"; //(imm); break;
		case 0xA4: return "LDY"; //(zp); break;
		case 0xB4: return "LDY"; //(zp_x); break;
		case 0xAC: return "LDY"; //(abs); break;
		case 0xBC: return "LDY"; //(abs_x); break;
		case 0x85: return "STA"; //(zp); break;
		case 0x95: return "STA"; //(zp_x); break;
		case 0x8D: return "STA"; //(abs); break;
		case 0x9D: return "STA"; //(abs_x); break;
		case 0x99: return "STA"; //(abs_y); break;
		case 0x81: return "STA"; //(idx_ind_x); break;
		case 0x91: return "STA"; //(ind_idx_y); break;
		case 0x86: return "STX"; //(zp); break;
		case 0x96: return "STX"; //(mode_zp_y); break;
		case 0x8E: return "STX"; //(abs); break;
		case 0x84: return "STY"; //(zp); break;
		case 0x94: return "STY"; //(zp_x); break;
		case 0x8C: return "STY"; //(abs); break;
		case 0xAA: return "TAX"; //break;
		case 0x8A: return "TXA"; //break;
		case 0xA8: return "TAY"; //break;
		case 0x98: return "TYA"; //break;
		case 0xCA: return "DEX"; //break;
		case 0xE8: return "INX"; //break;
		case 0x88: return "DEY"; //break;
		case 0xC8: return "INY"; //break;
		case 0x9A: return "TXS"; //break;
		case 0xBA: return "TSX"; //break;
		case 0x48: return "PHA"; //break;
		case 0x08: return "PHP"; //break;
		case 0x68: return "PLA"; //break;
		case 0x28: return "PLP"; //break;
		case 0xEA: return "NOP"; //break;
		default: return "???";
	}
}