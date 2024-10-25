#include <logger.h>

#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>

using namespace NES;

static const std::string operand_pat = "{{OPERAND}}";
static const std::string target_pat = "{{TARGET}}";
static const std::string sum_pat = "{{SUM}}";
static const std::string im_pat = "{{IM}}";

SystemLogger::SystemLogger(CPU &cpu, PPU &ppu, MemoryBus &bus)
    : cpu(cpu), ppu(ppu), bus(bus), logs() {}

uint16_t SystemLogger::bus_read16(uint16_t addr, bool zp = false) {
    // If we know this is a zero-page addr, wrap the most-significant bit
    // around zero-page bounds
    uint16_t h_addr = zp ? ((addr + 1) % 0x100) : (addr + 1);
    uint8_t l_data = bus.read(addr);
    uint8_t h_data = bus.read(h_addr);
    return (h_data << 8) | l_data;
}

std::string SystemLogger::log() {
    using namespace std;

    string line;
    stringstream ss;
    string op_templ;
    uint8_t op_len;
    uint8_t opcode = bus.read(cpu.PC);
    std::optional<AddressingMode> addr_mode = addr_mode_for_op(opcode);

    if (addr_mode.has_value())
        op_len = operand_len(addr_mode.value());
    else
        op_len = 0;

    // PC as a 4-char wide hex string.
    ss << setfill('0') << setw(4) << hex << (int)cpu.PC << "  ";
    line += string(ss.str());
    ss.str(string());

    // Current opcode as 2-char wide hex.
    ss << setfill('0') << setw(2) << hex << (int)opcode << " ";
    line += string(ss.str());
    ss.str(string());

    // Fill opcode parameters as 2-char wide hex values.
    if (op_len > 0) {
        for (int i = 0; i < op_len; i++) {
            uint8_t op8 = bus.read(cpu.PC + (uint8_t)1 + (uint8_t)i);
            ss << setfill('0') << setw(2) << hex << (int)op8 << " ";
            line += string(ss.str());
            ss.str(string());
        }
        for (int j = 2 - op_len; j > 0; j--) line += "   ";
    } else
        line += "      ";

    // Add sign if opcode is unofficial
    if (is_opcode_legal(opcode))
        line += " ";
    else
        line += "*";

    // Decode opcode to string.
    line += decode(opcode) + " ";

    // Pretty print parameter with addressing mode.
    if (addr_mode.has_value() && op_len > 0) {
        // uint16_t operand = 0;
        //  Get template for the mode.
        op_templ = templ_for_mode(addr_mode.value(), opcode);

        // Revert endianness.
        for (int i = op_len; i > 0; i--) {
            uint8_t op8 = bus.read(cpu.PC + (uint8_t)i);
            ss << setfill('0') << setw(2) << hex << (int)op8;
            // operand |= op8 << (i - 1) * 8;
        }

        // Replace template with the operand in little endian.
        size_t pos = op_templ.find(operand_pat);
        if (pos != std::string::npos)
            op_templ.replace(pos, operand_pat.length(), ss.str());
        ss.str(string());

        if (addr_mode.value() == idx_ind_x) {
            uint8_t opsum = bus.read(cpu.PC + 1) + cpu.X;
            ss << setfill('0') << setw(2) << hex << (int)opsum;
            op_templ.replace(op_templ.find(sum_pat), sum_pat.length(),
                             ss.str());
            ss.str(string());

            uint16_t imval =
                bus_read16((bus.read(cpu.PC + 1) + cpu.X) % 0x100, true);
            ss << setfill('0') << setw(4) << hex << imval;
            op_templ.replace(op_templ.find(im_pat), im_pat.length(), ss.str());
            ss.str(string());
        } else if (addr_mode.value() == ind_idx_y) {
            uint16_t zpaddr = bus_read16(bus.read(cpu.PC + 1), true);
            ss << setfill('0') << setw(4) << hex << (int)zpaddr;
            op_templ.replace(op_templ.find(sum_pat), sum_pat.length(),
                             ss.str());
            ss.str(string());

            uint16_t addrsum = zpaddr + cpu.Y;
            ss << setfill('0') << setw(4) << hex << (int)addrsum;
            op_templ.replace(op_templ.find(im_pat), im_pat.length(), ss.str());
            ss.str(string());
        } else if (addr_mode.value() == abs_x || addr_mode.value() == abs_y) {
            uint16_t sum = bus_read16(cpu.PC + 1);
            sum += addr_mode.value() == abs_x ? cpu.X : cpu.Y;
            ss << setfill('0') << setw(4) << hex << (int)sum;
            op_templ.replace(op_templ.find(sum_pat), sum_pat.length(),
                             ss.str());
            ss.str(string());
        } else if (addr_mode.value() == zp_x || addr_mode.value() == zp_y) {
            uint8_t sum = bus.read(cpu.PC + 1);
            sum += addr_mode.value() == zp_x ? cpu.X : cpu.Y;
            ss << setfill('0') << setw(2) << hex << (int)sum;
            op_templ.replace(op_templ.find(sum_pat), sum_pat.length(),
                             ss.str());
            ss.str(string());
        }

        uint8_t tgt_len = target_len(addr_mode.value(), opcode);
        if (tgt_len > 0) {
            uint16_t val = target_value(addr_mode.value());

            ss << setfill('0') << setw(tgt_len * 2) << hex << val;

            op_templ.replace(op_templ.find(target_pat), target_pat.length(),
                             ss.str());

            ss.str(string());
        }

        line += op_templ;

        // Add whitespace to fill 30 chars in total
        int wspace_len = 30 - (int)op_templ.length();
        for (int j = 0; j < wspace_len; j++) line += " ";
    } else
        for (int j = 0; j < 30; j++) line += " ";

    // CPU register status as 2-char wide hex.
    ss << "A:" << setfill('0') << setw(2) << hex << (int)cpu.A << " ";
    ss << "X:" << setfill('0') << setw(2) << hex << (int)cpu.X << " ";
    ss << "Y:" << setfill('0') << setw(2) << hex << (int)cpu.Y << " ";
    ss << "P:" << setfill('0') << setw(2) << hex << (int)cpu.P.status << " ";
    ss << "SP:" << setfill('0') << setw(2) << hex << (int)cpu.S << " ";
    ss << "PPU:" << setfill(' ') << setw(3) << right << dec << (int)ppu.scan_y
       << "," << setfill(' ') << setw(3) << right << dec << (int)ppu.scan_x
       << " ";
    ss << "CYC:" << dec << (int)cpu.cycles;
    line += string(ss.str());
    ss.str(string());

    // Convert to uppercase chars.
    transform(line.begin(), line.end(), line.begin(),
              [](char c) -> char { return (char)toupper(c); });

    // Push to log storage.
    // TODO: Add doing partial writes as the size of this can quickly get out
    // of hand
    logs.push_back(line);

    // Write line to output stream if set
    if (instr_ostream) instr_ostream.value().get() << line << endl;

    return line;
}

void SystemLogger::save() {
    std::ofstream fstream;

    if (log_filename)
        fstream.open(log_filename.value());
    else
        fstream.open("latest.log");

    for (const auto &str : logs) fstream << str << std::endl;

    fstream.close();
}

std::string SystemLogger::decode(uint8_t opcode) {
    switch (opcode) {
    case 0x0: return "BRK";
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
    case 0xE6: return "INC";
    case 0xF6: return "INC";
    case 0xEE: return "INC";
    case 0xFE: return "INC";
    // Unofficial
    case 0xA7: return "LAX";
    case 0xB7: return "LAX";
    case 0xAF: return "LAX";
    case 0xBF: return "LAX";
    case 0xA3: return "LAX";
    case 0xB3: return "LAX";
    case 0x87: return "SAX";
    case 0x97: return "SAX";
    case 0x8F: return "SAX";
    case 0x83: return "SAX";
    case 0xEB: return "SBC";
    case 0xC7: return "DCP";
    case 0xD7: return "DCP";
    case 0xCF: return "DCP";
    case 0xDF: return "DCP";
    case 0xDB: return "DCP";
    case 0xC3: return "DCP";
    case 0xD3: return "DCP";
    case 0xE7: return "ISB";
    case 0xF7: return "ISB";
    case 0xEF: return "ISB";
    case 0xFF: return "ISB";
    case 0xFB: return "ISB";
    case 0xE3: return "ISB";
    case 0xF3: return "ISB";
    case 0x07: return "SLO";
    case 0x17: return "SLO";
    case 0x0F: return "SLO";
    case 0x1F: return "SLO";
    case 0x1B: return "SLO";
    case 0x03: return "SLO";
    case 0x13: return "SLO";
    case 0x27: return "RLA";
    case 0x37: return "RLA";
    case 0x2F: return "RLA";
    case 0x3F: return "RLA";
    case 0x3B: return "RLA";
    case 0x23: return "RLA";
    case 0x33: return "RLA";
    case 0x47: return "SRE";
    case 0x57: return "SRE";
    case 0x4F: return "SRE";
    case 0x5F: return "SRE";
    case 0x5B: return "SRE";
    case 0x43: return "SRE";
    case 0x53: return "SRE";
    case 0x67: return "RRA";
    case 0x77: return "RRA";
    case 0x6F: return "RRA";
    case 0x7F: return "RRA";
    case 0x7B: return "RRA";
    case 0x63: return "RRA";
    case 0x73: return "RRA";
    case 0x1A:
    case 0x3A:
    case 0x5A:
    case 0x7A:
    case 0xDA:
    case 0xFA:
    /* 2-byte NOPs */
    case 0x80:
    case 0x82:
    case 0x89:
    case 0xC2:
    case 0xE2:
    case 0x04:
    case 0x44:
    case 0x64:
    case 0x14:
    case 0x34:
    case 0x54:
    case 0x74:
    case 0xD4:
    case 0xF4:
    /* 3-byte NOPs */
    case 0x0C:
    case 0x1C:
    case 0x3C:
    case 0x5C:
    case 0x7C:
    case 0xDC:
    case 0xFC: return "NOP";
    default: return "???";
    }
}

bool SystemLogger::is_opcode_legal(uint8_t opcode) {
    switch (opcode) {
    /* LAX */
    case 0xA7:
    case 0xB7:
    case 0xAF:
    case 0xBF:
    case 0xA3:
    case 0xB3:
    /* SAX */
    case 0x87:
    case 0x97:
    case 0x8F:
    case 0x83:
    /* USBC / *SBC */
    case 0xEB:
    /* DCP */
    case 0xC7:
    case 0xD7:
    case 0xCF:
    case 0xDF:
    case 0xDB:
    case 0xC3:
    case 0xD3:
    /* ISC / ISB / INS */
    case 0xE7:
    case 0xF7:
    case 0xEF:
    case 0xFF:
    case 0xFB:
    case 0xE3:
    case 0xF3:
    /* SLO / ASO */
    case 0x07:
    case 0x17:
    case 0x0F:
    case 0x1F:
    case 0x1B:
    case 0x03:
    case 0x13:
    /* RLA */
    case 0x27:
    case 0x37:
    case 0x2F:
    case 0x3F:
    case 0x3B:
    case 0x23:
    case 0x33:
    /* SRE */
    case 0x47:
    case 0x57:
    case 0x4F:
    case 0x5F:
    case 0x5B:
    case 0x43:
    case 0x53:
    /* RRA */
    case 0x67:
    case 0x77:
    case 0x6F:
    case 0x7F:
    case 0x7B:
    case 0x63:
    case 0x73:
    /* 1-byte NOPs */
    case 0x1A:
    case 0x3A:
    case 0x5A:
    case 0x7A:
    case 0xDA:
    case 0xFA:
    /* 2-byte NOPs */
    case 0x80:
    case 0x82:
    case 0x89:
    case 0xC2:
    case 0xE2:
    case 0x04:
    case 0x44:
    case 0x64:
    case 0x14:
    case 0x34:
    case 0x54:
    case 0x74:
    case 0xD4:
    case 0xF4:
    /* 3-byte NOPs */
    case 0x0C:
    case 0x1C:
    case 0x3C:
    case 0x5C:
    case 0x7C:
    case 0xDC:
    case 0xFC: return false;
    default: return true;
    }
}

std::optional<AddressingMode> SystemLogger::addr_mode_for_op(uint8_t opcode) {
    switch (opcode) {
    // De facto mode is relative for each conditional branch opcode.
    case 0x10: return {AddressingMode::rel};
    case 0x30: return {AddressingMode::rel};
    case 0x50: return {AddressingMode::rel};
    case 0x70: return {AddressingMode::rel};
    case 0x90: return {AddressingMode::rel};
    case 0xB0: return {AddressingMode::rel};
    case 0xD0: return {AddressingMode::rel};
    case 0xF0: return {AddressingMode::rel};
    case 0x4C: return {AddressingMode::abs};
    case 0x6C: return {AddressingMode::ind};
    case 0x20: return {AddressingMode::abs};
    case 0x60: return std::nullopt;
    case 0x40: return std::nullopt;
    case 0x69: return {AddressingMode::imm};
    case 0x65: return {AddressingMode::zp};
    case 0x75: return {AddressingMode::zp_x};
    case 0x6D: return {AddressingMode::abs};
    case 0x7D: return {AddressingMode::abs_x};
    case 0x79: return {AddressingMode::abs_y};
    case 0x61: return {AddressingMode::idx_ind_x};
    case 0x71: return {AddressingMode::ind_idx_y};
    case 0x29: return {AddressingMode::imm};
    case 0x25: return {AddressingMode::zp};
    case 0x35: return {AddressingMode::zp_x};
    case 0x2D: return {AddressingMode::abs};
    case 0x3D: return {AddressingMode::abs_x};
    case 0x39: return {AddressingMode::abs_y};
    case 0x21: return {AddressingMode::idx_ind_x};
    case 0x31: return {AddressingMode::ind_idx_y};
    case 0x0A: return std::nullopt;
    case 0x06: return {AddressingMode::zp};
    case 0x16: return {AddressingMode::zp_x};
    case 0x0E: return {AddressingMode::abs};
    case 0x1E: return {AddressingMode::abs_x};
    case 0x24: return {AddressingMode::zp};
    case 0x2C: return {AddressingMode::abs};
    case 0xC9: return {AddressingMode::imm};
    case 0xC5: return {AddressingMode::zp};
    case 0xD5: return {AddressingMode::zp_x};
    case 0xCD: return {AddressingMode::abs};
    case 0xDD: return {AddressingMode::abs_x};
    case 0xD9: return {AddressingMode::abs_y};
    case 0xC1: return {AddressingMode::idx_ind_x};
    case 0xD1: return {AddressingMode::ind_idx_y};
    case 0xE0: return {AddressingMode::imm};
    case 0xE4: return {AddressingMode::zp};
    case 0xEC: return {AddressingMode::abs};
    case 0xC0: return {AddressingMode::imm};
    case 0xC4: return {AddressingMode::zp};
    case 0xCC: return {AddressingMode::abs};
    case 0xC6: return {AddressingMode::zp};
    case 0xD6: return {AddressingMode::zp_x};
    case 0xCE: return {AddressingMode::abs};
    case 0xDE: return {AddressingMode::abs_x};
    case 0x49: return {AddressingMode::imm};
    case 0x45: return {AddressingMode::zp};
    case 0x55: return {AddressingMode::zp_x};
    case 0x4D: return {AddressingMode::abs};
    case 0x5D: return {AddressingMode::abs_x};
    case 0x59: return {AddressingMode::abs_y};
    case 0x41: return {AddressingMode::idx_ind_x};
    case 0x51: return {AddressingMode::ind_idx_y};
    case 0x18: return std::nullopt;
    case 0x38: return std::nullopt;
    case 0x58: return std::nullopt;
    case 0x78: return std::nullopt;
    case 0xB8: return std::nullopt;
    case 0xD8: return std::nullopt;
    case 0xF8: return std::nullopt;
    case 0x4A: return std::nullopt;
    case 0x46: return {AddressingMode::zp};
    case 0x56: return {AddressingMode::zp_x};
    case 0x4E: return {AddressingMode::abs};
    case 0x5E: return {AddressingMode::abs_x};
    case 0x09: return {AddressingMode::imm};
    case 0x05: return {AddressingMode::zp};
    case 0x15: return {AddressingMode::zp_x};
    case 0x0D: return {AddressingMode::abs};
    case 0x1D: return {AddressingMode::abs_x};
    case 0x19: return {AddressingMode::abs_y};
    case 0x01: return {AddressingMode::idx_ind_x};
    case 0x11: return {AddressingMode::ind_idx_y};
    case 0x2A: return std::nullopt;
    case 0x26: return {AddressingMode::zp};
    case 0x36: return {AddressingMode::zp_x};
    case 0x2E: return {AddressingMode::abs};
    case 0x3E: return {AddressingMode::abs_x};
    case 0x6A: return std::nullopt;
    case 0x66: return {AddressingMode::zp};
    case 0x76: return {AddressingMode::zp_x};
    case 0x6E: return {AddressingMode::abs};
    case 0x7E: return {AddressingMode::abs_x};
    case 0xE9: return {AddressingMode::imm};
    case 0xE5: return {AddressingMode::zp};
    case 0xF5: return {AddressingMode::zp_x};
    case 0xED: return {AddressingMode::abs};
    case 0xFD: return {AddressingMode::abs_x};
    case 0xF9: return {AddressingMode::abs_y};
    case 0xE1: return {AddressingMode::idx_ind_x};
    case 0xF1: return {AddressingMode::ind_idx_y};
    case 0xA9: return {AddressingMode::imm};
    case 0xA5: return {AddressingMode::zp};
    case 0xB5: return {AddressingMode::zp_x};
    case 0xAD: return {AddressingMode::abs};
    case 0xBD: return {AddressingMode::abs_x};
    case 0xB9: return {AddressingMode::abs_y};
    case 0xA1: return {AddressingMode::idx_ind_x};
    case 0xB1: return {AddressingMode::ind_idx_y};
    case 0xA2: return {AddressingMode::imm};
    case 0xA6: return {AddressingMode::zp};
    case 0xB6: return {AddressingMode::zp_y};
    case 0xAE: return {AddressingMode::abs};
    case 0xBE: return {AddressingMode::abs_y};
    case 0xA0: return {AddressingMode::imm};
    case 0xA4: return {AddressingMode::zp};
    case 0xB4: return {AddressingMode::zp_x};
    case 0xAC: return {AddressingMode::abs};
    case 0xBC: return {AddressingMode::abs_x};
    case 0x85: return {AddressingMode::zp};
    case 0x95: return {AddressingMode::zp_x};
    case 0x8D: return {AddressingMode::abs};
    case 0x9D: return {AddressingMode::abs_x};
    case 0x99: return {AddressingMode::abs_y};
    case 0x81: return {AddressingMode::idx_ind_x};
    case 0x91: return {AddressingMode::ind_idx_y};
    case 0x86: return {AddressingMode::zp};
    case 0x96: return {AddressingMode::zp_y};
    case 0x8E: return {AddressingMode::abs};
    case 0x84: return {AddressingMode::zp};
    case 0x94: return {AddressingMode::zp_x};
    case 0x8C: return {AddressingMode::abs};
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
    case 0xE6: return {AddressingMode::zp};
    case 0xF6: return {AddressingMode::zp_x};
    case 0xEE: return {AddressingMode::abs};
    case 0xFE: return {AddressingMode::abs_x};
    // Unofficial
    /* LAX */
    case 0xA7: return {AddressingMode::zp};
    case 0xB7: return {AddressingMode::zp_y};
    case 0xAF: return {AddressingMode::abs};
    case 0xBF: return {AddressingMode::abs_y};
    case 0xA3: return {AddressingMode::idx_ind_x};
    case 0xB3: return {AddressingMode::ind_idx_y};
    /* SAX */
    case 0x87: return {AddressingMode::zp};
    case 0x97: return {AddressingMode::zp_y};
    case 0x8F: return {AddressingMode::abs};
    case 0x83: return {AddressingMode::idx_ind_x};
    /* USBC */
    case 0xEB: return {AddressingMode::imm};
    /* DCP */
    case 0xC7: return {AddressingMode::zp};
    case 0xD7: return {AddressingMode::zp_x};
    case 0xCF: return {AddressingMode::abs};
    case 0xDF: return {AddressingMode::abs_x};
    case 0xDB: return {AddressingMode::abs_y};
    case 0xC3: return {AddressingMode::idx_ind_x};
    case 0xD3: return {AddressingMode::ind_idx_y};
    /* ISC */
    case 0xE7: return {AddressingMode::zp};
    case 0xF7: return {AddressingMode::zp_x};
    case 0xEF: return {AddressingMode::abs};
    case 0xFF: return {AddressingMode::abs_x};
    case 0xFB: return {AddressingMode::abs_y};
    case 0xE3: return {AddressingMode::idx_ind_x};
    case 0xF3: return {AddressingMode::ind_idx_y};
    /* SLO */
    case 0x07: return {AddressingMode::zp};
    case 0x17: return {AddressingMode::zp_x};
    case 0x0F: return {AddressingMode::abs};
    case 0x1F: return {AddressingMode::abs_x};
    case 0x1B: return {AddressingMode::abs_y};
    case 0x03: return {AddressingMode::idx_ind_x};
    case 0x13: return {AddressingMode::ind_idx_y};
    /* RLA */
    case 0x27: return {AddressingMode::zp};
    case 0x37: return {AddressingMode::zp_x};
    case 0x2F: return {AddressingMode::abs};
    case 0x3F: return {AddressingMode::abs_x};
    case 0x3B: return {AddressingMode::abs_y};
    case 0x23: return {AddressingMode::idx_ind_x};
    case 0x33: return {AddressingMode::ind_idx_y};
    /* SRE */
    case 0x47: return {AddressingMode::zp};
    case 0x57: return {AddressingMode::zp_x};
    case 0x4F: return {AddressingMode::abs};
    case 0x5F: return {AddressingMode::abs_x};
    case 0x5B: return {AddressingMode::abs_y};
    case 0x43: return {AddressingMode::idx_ind_x};
    case 0x53: return {AddressingMode::ind_idx_y};
    /* RRA */
    case 0x67: return {AddressingMode::zp};
    case 0x77: return {AddressingMode::zp_x};
    case 0x6F: return {AddressingMode::abs};
    case 0x7F: return {AddressingMode::abs_x};
    case 0x7B: return {AddressingMode::abs_y};
    case 0x63: return {AddressingMode::idx_ind_x};
    case 0x73: return {AddressingMode::ind_idx_y};
    /* NOP */
    case 0x1A:
    case 0x3A:
    case 0x5A:
    case 0x7A:
    case 0xDA:
    case 0xFA: return std::nullopt;
    case 0x80:
    case 0x82:
    case 0x89:
    case 0xC2:
    case 0xE2: return {AddressingMode::imm};
    case 0x04:
    case 0x44:
    case 0x64: return {AddressingMode::zp};
    case 0x14:
    case 0x34:
    case 0x54:
    case 0x74:
    case 0xD4:
    case 0xF4: return {AddressingMode::zp_x};
    case 0x0C: return {AddressingMode::abs};
    case 0x1C:
    case 0x3C:
    case 0x5C:
    case 0x7C:
    case 0xDC:
    case 0xFC: return {AddressingMode::abs_x};
    default: return std::nullopt;
    }
}

std::string SystemLogger::templ_for_mode(AddressingMode addr_mode,
                                         uint8_t opcode) {
    switch (addr_mode) {
    case rel: return "$" + target_pat;
    case abs:
        return opcode == 0x4C || opcode == 0x20
                   ? "$" + operand_pat
                   : "$" + operand_pat + " = " + target_pat;
    case abs_x:
        return "$" + operand_pat + ",X @ " + sum_pat + " = " + target_pat;
    case abs_y:
        return "$" + operand_pat + ",Y @ " + sum_pat + " = " + target_pat;
    case imm:  // imd
        return "#$" + operand_pat;
    case zp: return "$" + operand_pat + " = " + target_pat;
    case zp_x:
        return "$" + operand_pat + ",X @ " + sum_pat + " = " + target_pat;
    case zp_y:
        return "$" + operand_pat + ",Y @ " + sum_pat + " = " + target_pat;
    case idx_ind_x:  // ndx
        return "($" + operand_pat + ",X) @ " + sum_pat + " = " + im_pat +
               " = " + target_pat;
    case ind_idx_y:  // ndy
        return "($" + operand_pat + "),Y = " + sum_pat + " @ " + im_pat +
               " = " + target_pat;
    case ind: return "($" + operand_pat + ") = " + target_pat;
    default: return "";
    }
}

uint8_t SystemLogger::operand_len(NES::AddressingMode addr_mode) {
    switch (addr_mode) {
    case abs:
    case abs_x:
    case abs_y:
    case ind: return 2;
    case rel:
    case zp:
    case zp_x:
    case zp_y:
    case imm:
    case idx_ind_x:
    case ind_idx_y: return 1;
    default: return 0;
    }
}

uint8_t SystemLogger::target_len(NES::AddressingMode addr_mode,
                                 uint8_t opcode) {
    switch (addr_mode) {
    case rel:
    case ind: return 2;
    case zp:
    case zp_x:
    case zp_y:
    case idx_ind_x:
    case ind_idx_y: return 1;
    case abs: return opcode == 0x4C || opcode == 0x20 ? 0 : 1;
    case abs_x:
    case abs_y: return 1;
    default: return 0;
    }
}

uint16_t SystemLogger::target_value(NES::AddressingMode addr_mode) {
    switch (addr_mode) {
    case rel:
        uint8_t rel_op;
        uint8_t pc_l;
        bool cross;
        uint16_t target;

        rel_op = bus.read(cpu.PC + 1);
        pc_l = uint8_t((cpu.PC + 2) & 0xFF);
        cross = uint16_t(pc_l) + rel_op >= 0x100;
        pc_l += rel_op;

        target = ((cpu.PC + 2) & 0xFF00) | (uint16_t)pc_l;
        if ((rel_op & 0x80) && !cross) {
            target -= 0x100;
        } else if (!(rel_op & 0x80) && cross) {
            target += 0x100;
        }

        return target;
    case abs: return (uint16_t)bus.read(bus_read16((uint16_t)(cpu.PC + 1)));
    case abs_x:
    case abs_y:
        uint16_t absaddr;
        absaddr = bus_read16(cpu.PC + 1);
        absaddr += addr_mode == abs_x ? cpu.X : cpu.Y;
        return (uint16_t)bus.read(absaddr);
    case ind:
        uint16_t h_addr, l_addr;
        l_addr = bus_read16((uint16_t)(cpu.PC + 1));
        h_addr = (l_addr % 0x100 == 0xFF) ? (uint16_t)(l_addr - l_addr % 0x100)
                                          : (uint16_t)(l_addr + 1);
        return (uint16_t)(bus.read(h_addr)) << 8 | bus.read(l_addr);
    case zp:
    case zp_x:
    case zp_y:
        uint8_t zp_addr;
        zp_addr = bus.read(cpu.PC + 1);
        if (addr_mode == zp_x) zp_addr += cpu.X;
        if (addr_mode == zp_y) zp_addr += cpu.Y;
        return (uint16_t)bus.read(zp_addr);
    case idx_ind_x:
        uint16_t imx_addr;
        imx_addr = bus_read16((bus.read(cpu.PC + 1) + cpu.X) % 0x100, true);
        return (uint16_t)bus.read(imx_addr);
    case ind_idx_y:
        uint16_t imy_addr;
        imy_addr = bus_read16(bus.read(cpu.PC + 1), true) + cpu.Y;
        return (uint16_t)bus.read(imy_addr);
    default: return std::numeric_limits<uint16_t>::max();
    }
}
