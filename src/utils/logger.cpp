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
	ss << std::setfill('0') << std::setw(4) << std::hex << (int)cpu.PC
		<< "  ";
	line += std::string(ss.str());
	ss.str(std::string());
	
	// Current opcode as hex.
	opcode = (int)bus.read(cpu.PC);
	ss << std::setfill('0') << std::setw(2) << std::hex << opcode << " ";
	line += std::string(ss.str());
	ss.str(std::string());
	
	// TODO: Fill opcode parameters here instead of empty space.
	line += "       ";
	
	// Decode opcode to string.
	line += decode(opcode);
	
	// TODO: Decode opcode param / addressing mode here.
	line += "                             ";
	
	// CPU register status.
	ss << "A:" << std::setfill('0') << std::setw(2) << std::hex
		<< (int)cpu.A << " ";
	ss << "X:" << std::setfill('0') << std::setw(2) << std::hex
		<< (int)cpu.X << " ";
	ss << "Y:" << std::setfill('0') << std::setw(2) << std::hex
	   	<< (int)cpu.Y << " ";
	ss << "P:" << std::setfill('0') << std::setw(2) << std::hex
	   	<< (int)cpu.P.status << " ";
	ss << "SP:" << std::setfill('0') << std::setw(2) << std::hex
	   	<< (int)cpu.S << " ";
	ss << "PPU:???,???" << " ";
	ss << "CYC:" << std::dec << (int)cpu.cycles;
	line += std::string(ss.str());
	ss.str(std::string());
	
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
		case 0x4C: return "JMP";
		case 0x6C: return "JMP";
		case 0x20: return "JSR";
		case 0x60: return "RTS";
		case 0x40: return "RTI";
		case 0x69: return "ADC";
		case 0x65: return "ADC";
		case 0x75: return "ADC";
		case 0x6D: return "ADC";
		case 0x7D: return "ADC";
		case 0x79: return "ADC";
		case 0x61: return "ADC";
		case 0x71: return "ADC";
		case 0x29: return "AND";
		case 0x25: return "AND";
		case 0x35: return "AND";
		case 0x2D: return "AND";
		case 0x3D: return "AND";
		case 0x39: return "AND";
		case 0x21: return "AND";
		case 0x31: return "AND";
		case 0x0A: return "ASL A";
		case 0x06: return "ASL";
		case 0x16: return "ASL";
		case 0x0E: return "ASL";
		case 0x1E: return "ASL";
		case 0x24: return "BIT";
		case 0x2C: return "BIT";
		case 0xC9: return "CMP";
		case 0xC5: return "CMP";
		case 0xD5: return "CMP";
		case 0xCD: return "CMP";
		case 0xDD: return "CMP";
		case 0xD9: return "CMP";
		case 0xC1: return "CMP";
		case 0xD1: return "CMP";
		case 0xE0: return "CPX";
		case 0xE4: return "CPX";
		case 0xEC: return "CPX";
		case 0xC0: return "CPY";
		case 0xC4: return "CPY";
		case 0xCC: return "CPY";
		case 0xC6: return "DEC";
		case 0xD6: return "DEC";
		case 0xCE: return "DEC";
		case 0xDE: return "DEC";
		case 0x49: return "EOR";
		case 0x45: return "EOR";
		case 0x55: return "EOR";
		case 0x4D: return "EOR";
		case 0x5D: return "EOR";
		case 0x59: return "EOR";
		case 0x41: return "EOR";
		case 0x51: return "EOR";
		case 0x18: return "CLC";
		case 0x38: return "SEC";
		case 0x58: return "CLI";
		case 0x78: return "SEI";
		case 0xB8: return "CLV";
		case 0xD8: return "CLD";
		case 0xF8: return "SED";
		case 0x4A: return "LSR A";
		case 0x46: return "LSR";
		case 0x56: return "LSR";
		case 0x4E: return "LSR";
		case 0x5E: return "LSR";
		case 0x09: return "ORA";
		case 0x05: return "ORA";
		case 0x15: return "ORA";
		case 0x0D: return "ORA";
		case 0x1D: return "ORA";
		case 0x19: return "ORA";
		case 0x01: return "ORA";
		case 0x11: return "ORA";
		case 0x2A: return "ROL A";
		case 0x26: return "ROL";
		case 0x36: return "ROL";
		case 0x2E: return "ROL";
		case 0x3E: return "ROL";
		case 0x6A: return "ROR A";
		case 0x66: return "ROR";
		case 0x76: return "ROR";
		case 0x6E: return "ROR";
		case 0x7E: return "ROR";
		case 0xE9: return "SBC";
		case 0xE5: return "SBC";
		case 0xF5: return "SBC";
		case 0xED: return "SBC";
		case 0xFD: return "SBC";
		case 0xF9: return "SBC";
		case 0xE1: return "SBC";
		case 0xF1: return "SBC";
		case 0xA9: return "LDA";
		case 0xA5: return "LDA";
		case 0xB5: return "LDA";
		case 0xAD: return "LDA";
		case 0xBD: return "LDA";
		case 0xB9: return "LDA";
		case 0xA1: return "LDA";
		case 0xB1: return "LDA";
		case 0xA2: return "LDX";
		case 0xA6: return "LDX";
		case 0xB6: return "LDX";
		case 0xAE: return "LDX";
		case 0xBE: return "LDX";
		case 0xA0: return "LDY";
		case 0xA4: return "LDY";
		case 0xB4: return "LDY";
		case 0xAC: return "LDY";
		case 0xBC: return "LDY";
		case 0x85: return "STA";
		case 0x95: return "STA";
		case 0x8D: return "STA";
		case 0x9D: return "STA";
		case 0x99: return "STA";
		case 0x81: return "STA";
		case 0x91: return "STA";
		case 0x86: return "STX";
		case 0x96: return "STX";
		case 0x8E: return "STX";
		case 0x84: return "STY";
		case 0x94: return "STY";
		case 0x8C: return "STY";
		case 0xAA: return "TAX";
		case 0x8A: return "TXA";
		case 0xA8: return "TAY";
		case 0x98: return "TYA";
		case 0xCA: return "DEX";
		case 0xE8: return "INX";
		case 0x88: return "DEY";
		case 0xC8: return "INY";
		case 0x9A: return "TXS";
		case 0xBA: return "TSX";
		case 0x48: return "PHA";
		case 0x08: return "PHP";
		case 0x68: return "PLA";
		case 0x28: return "PLP";
		case 0xEA: return "NOP";
		default: return "???";
	}
}

std::optional<NES::AddressingMode> NES::CPULogger::addr_mode(uint8_t opcode)
{
	switch (opcode) {
		// De facto mode is relative for each conditional branch opcode
		case 0x10: return { NES::AddressingMode::imm };
		case 0x30: return { NES::AddressingMode::imm };
		case 0x50: return { NES::AddressingMode::imm };
		case 0x70: return { NES::AddressingMode::imm };
		case 0x90: return { NES::AddressingMode::imm };
		case 0xB0: return { NES::AddressingMode::imm };
		case 0xD0: return { NES::AddressingMode::imm };
		case 0xF0: return { NES::AddressingMode::imm };
		case 0x4C: return { NES::AddressingMode::abs };
		case 0x6C: return { NES::AddressingMode::ind };
		case 0x20: return { NES::AddressingMode::abs };
		case 0x60: return std::nullopt;
		case 0x40: return std::nullopt;
		case 0x69: return { NES::AddressingMode::imm };
		case 0x65: return { NES::AddressingMode::zp };
		case 0x75: return { NES::AddressingMode::zp_x };
		case 0x6D: return { NES::AddressingMode::abs };
		case 0x7D: return { NES::AddressingMode::abs_x };
		case 0x79: return { NES::AddressingMode::abs_y };
		case 0x61: return { NES::AddressingMode::idx_ind_x };
		case 0x71: return { NES::AddressingMode::ind_idx_y };
		case 0x29: return { NES::AddressingMode::imm };
		case 0x25: return { NES::AddressingMode::zp };
		case 0x35: return { NES::AddressingMode::zp_x };
		case 0x2D: return { NES::AddressingMode::abs };
		case 0x3D: return { NES::AddressingMode::abs_x };
		case 0x39: return { NES::AddressingMode::abs_y };
		case 0x21: return { NES::AddressingMode::idx_ind_x };
		case 0x31: return { NES::AddressingMode::ind_idx_y };
		case 0x0A: return std::nullopt;
		case 0x06: return { NES::AddressingMode::zp };
		case 0x16: return { NES::AddressingMode::zp_x };
		case 0x0E: return { NES::AddressingMode::abs };
		case 0x1E: return { NES::AddressingMode::abs_x };
		case 0x24: return { NES::AddressingMode::zp };
		case 0x2C: return { NES::AddressingMode::abs };
		case 0xC9: return { NES::AddressingMode::imm };
		case 0xC5: return { NES::AddressingMode::zp };
		case 0xD5: return { NES::AddressingMode::zp_x };
		case 0xCD: return { NES::AddressingMode::abs };
		case 0xDD: return { NES::AddressingMode::abs_x };
		case 0xD9: return { NES::AddressingMode::abs_y };
		case 0xC1: return { NES::AddressingMode::idx_ind_x };
		case 0xD1: return { NES::AddressingMode::ind_idx_y };
		case 0xE0: return { NES::AddressingMode::imm };
		case 0xE4: return { NES::AddressingMode::zp };
		case 0xEC: return { NES::AddressingMode::abs };
		case 0xC0: return { NES::AddressingMode::imm };
		case 0xC4: return { NES::AddressingMode::zp };
		case 0xCC: return { NES::AddressingMode::abs };
		case 0xC6: return { NES::AddressingMode::zp };
		case 0xD6: return { NES::AddressingMode::zp_x };
		case 0xCE: return { NES::AddressingMode::abs };
		case 0xDE: return { NES::AddressingMode::abs_x };
		case 0x49: return { NES::AddressingMode::imm };
		case 0x45: return { NES::AddressingMode::zp };
		case 0x55: return { NES::AddressingMode::zp_x };
		case 0x4D: return { NES::AddressingMode::abs };
		case 0x5D: return { NES::AddressingMode::abs_x };
		case 0x59: return { NES::AddressingMode::abs_y };
		case 0x41: return { NES::AddressingMode::idx_ind_x };
		case 0x51: return { NES::AddressingMode::ind_idx_y };
		case 0x18: return std::nullopt;
		case 0x38: return std::nullopt;
		case 0x58: return std::nullopt;
		case 0x78: return std::nullopt;
		case 0xB8: return std::nullopt;
		case 0xD8: return std::nullopt;
		case 0xF8: return std::nullopt;
		case 0x4A: return std::nullopt;
		case 0x46: return { NES::AddressingMode::zp };
		case 0x56: return { NES::AddressingMode::zp_x };
		case 0x4E: return { NES::AddressingMode::abs };
		case 0x5E: return { NES::AddressingMode::abs_x };
		case 0x09: return { NES::AddressingMode::imm };
		case 0x05: return { NES::AddressingMode::zp };
		case 0x15: return { NES::AddressingMode::zp_x };
		case 0x0D: return { NES::AddressingMode::abs };
		case 0x1D: return { NES::AddressingMode::abs_x };
		case 0x19: return { NES::AddressingMode::abs_y };
		case 0x01: return { NES::AddressingMode::idx_ind_x };
		case 0x11: return { NES::AddressingMode::ind_idx_y };
		case 0x2A: return std::nullopt;
		case 0x26: return { NES::AddressingMode::zp };
		case 0x36: return { NES::AddressingMode::zp_x };
		case 0x2E: return { NES::AddressingMode::abs };
		case 0x3E: return { NES::AddressingMode::abs_x };
		case 0x6A: return std::nullopt;
		case 0x66: return { NES::AddressingMode::zp };
		case 0x76: return { NES::AddressingMode::zp_x };
		case 0x6E: return { NES::AddressingMode::abs };
		case 0x7E: return { NES::AddressingMode::abs_x };
		case 0xE9: return { NES::AddressingMode::imm };
		case 0xE5: return { NES::AddressingMode::zp };
		case 0xF5: return { NES::AddressingMode::zp_x };
		case 0xED: return { NES::AddressingMode::abs };
		case 0xFD: return { NES::AddressingMode::abs_x };
		case 0xF9: return { NES::AddressingMode::abs_y };
		case 0xE1: return { NES::AddressingMode::idx_ind_x };
		case 0xF1: return { NES::AddressingMode::ind_idx_y };
		case 0xA9: return { NES::AddressingMode::imm };
		case 0xA5: return { NES::AddressingMode::zp };
		case 0xB5: return { NES::AddressingMode::zp_x };
		case 0xAD: return { NES::AddressingMode::abs };
		case 0xBD: return { NES::AddressingMode::abs_x };
		case 0xB9: return { NES::AddressingMode::abs_y };
		case 0xA1: return { NES::AddressingMode::idx_ind_x };
		case 0xB1: return { NES::AddressingMode::ind_idx_y };
		case 0xA2: return { NES::AddressingMode::imm };
		case 0xA6: return { NES::AddressingMode::zp };
		case 0xB6: return { NES::AddressingMode::zp_y };
		case 0xAE: return { NES::AddressingMode::abs };
		case 0xBE: return { NES::AddressingMode::abs_y };
		case 0xA0: return { NES::AddressingMode::imm };
		case 0xA4: return { NES::AddressingMode::zp };
		case 0xB4: return { NES::AddressingMode::zp_x };
		case 0xAC: return { NES::AddressingMode::abs };
		case 0xBC: return { NES::AddressingMode::abs_x };
		case 0x85: return { NES::AddressingMode::zp };
		case 0x95: return { NES::AddressingMode::zp_x };
		case 0x8D: return { NES::AddressingMode::abs };
		case 0x9D: return { NES::AddressingMode::abs_x };
		case 0x99: return { NES::AddressingMode::abs_y };
		case 0x81: return { NES::AddressingMode::idx_ind_x };
		case 0x91: return { NES::AddressingMode::ind_idx_y };
		case 0x86: return { NES::AddressingMode::zp };
		case 0x96: return { NES::AddressingMode::zp_y };
		case 0x8E: return { NES::AddressingMode::abs };
		case 0x84: return { NES::AddressingMode::zp };
		case 0x94: return { NES::AddressingMode::zp_x };
		case 0x8C: return { NES::AddressingMode::abs };
		case 0xAA: return std::nullopt;
		case 0x8A: return std::nullopt;
		case 0xA8: return std::nullopt;
		case 0x98: return std::nullopt;
		case 0xCA: return std::nullopt;
		case 0xE8: return std::nullopt;
		case 0x88: return std::nullopt;
		case 0xC8: return std::nullopt;
		case 0x9A: return std::nullopt;
		case 0xBA: return std::nullopt;
		case 0x48: return std::nullopt;
		case 0x08: return std::nullopt;
		case 0x68: return std::nullopt;
		case 0x28: return std::nullopt;
		case 0xEA: return std::nullopt;
		default: return std::nullopt;
	}
}