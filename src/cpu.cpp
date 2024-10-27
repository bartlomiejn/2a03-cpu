#include <bus.h>
#include <cpu.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>

// Detailed cycle counting
// Example 1: LDA (Load Accumulator) Instruction
//
// Immediate Addressing Mode (LDA #$NN)
//
//     Cycles Required: 2
//     Cycle Breakdown:
//         Cycle 1:
//             Fetch opcode (LDA #), increment PC.
//         Cycle 2:
//             Fetch immediate value (#$NN), increment PC.
//             Load immediate value into the Accumulator.
//
// Zero Page Addressing Mode (LDA $NN)
//
//     Cycles Required: 3
//     Cycle Breakdown:
//         Cycle 1:
//             Fetch opcode (LDA $), increment PC.
//         Cycle 2:
//             Fetch zero-page address ($NN), increment PC.
//         Cycle 3:
//             Read value from zero-page address, load into Accumulator.
//
// Absolute Addressing Mode (LDA $NNNN)
//
//     Cycles Required: 4
//     Cycle Breakdown:
//         Cycle 1:
//             Fetch opcode, increment PC.
//         Cycle 2:
//             Fetch low byte of address ($NN), increment PC.
//         Cycle 3:
//             Fetch high byte of address ($NN), increment PC.
//         Cycle 4:
//             Read value from the full address, load into Accumulator.
//
// Example 2: STA (Store Accumulator) Instruction
//
// Zero Page Addressing Mode (STA $NN)
//
//     Cycles Required: 3
//     Cycle Breakdown:
//         Cycle 1:
//             Fetch opcode (STA $), increment PC.
//         Cycle 2:
//             Fetch zero-page address ($NN), increment PC.
//         Cycle 3:
//             Write Accumulator to zero-page address.
//
// Example 3: Branch Instructions (e.g., BEQ, BNE)
//
// Branch if Equal (BEQ $NN)
//
//     Cycles Required: 2 (not taken), 3 (taken), or 4 (taken and page crossed)
//     Cycle Breakdown:
//         Cycle 1:
//             Fetch opcode (BEQ $), increment PC.
//         Cycle 2:
//             Fetch relative address ($NN), increment PC.
//             If the condition is false, instruction ends here.
//         Cycle 3 (If Branch Taken):
//             Add relative address to PC low byte.
//             If no page crossing, proceed to next instruction.
//         Cycle 4 (If Page Crossed):
//             Adjust PC high byte due to page crossing.

using namespace NES;

CPU::CPU(NES::MemoryBusIntf *bus) : bus(bus) {
    A = 0x0;
    X = 0x0;
    Y = 0x0;
    S = 0xFD;
    P.status = 0x24;
    cycles = 0;
    IRQ = NMI = false;
}

void CPU::power() {
    A = 0x0;
    X = 0x0;
    Y = 0x0;
    S = 0xFD;
    P.status = 0x24;
    cycles = 0;
    IRQ = NMI = false;

    // for (uint16_t i = 0x4000; i <= 0x4013; i++) write(i, 0x0);
    // write(0x4015, 0x0);  // All channels disabled
    // write(0x4017, 0x0);  // Frame IRQ enabled

    // TODO: Rest of power up logic
    // All 15 bits of noise channel LFSR = $0000[4]. The first time the LFSR
    // is clocked from the all-0s state, it will shift in a 1.

    reset();
}

void CPU::reset() { interrupt(i_reset); }

uint16_t CPU::execute() {
    uint32_t initial_cyc = cycles;
    opcode = read(PC++);
    switch (opcode) {
    case 0x0: BRK(); break;
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
    // Unofficial
    case 0xA7: LAX(zp); break;
    case 0xB7: LAX(zp_y); break;
    case 0xAF: LAX(abs); break;
    case 0xBF: LAX(abs_y); break;
    case 0xA3: LAX(idx_ind_x); break;
    case 0xB3: LAX(ind_idx_y); break;
    case 0xAB: LXA(); break;
    case 0x87: SAX(zp); break;
    case 0x97: SAX(zp_y); break;
    case 0x8F: SAX(abs); break;
    case 0x83: SAX(idx_ind_x); break;
    case 0xCB: SBX(); break;
    case 0xEB: USBC(); break;
    case 0xC7: DCP(zp); break;
    case 0xD7: DCP(zp_x); break;
    case 0xCF: DCP(abs); break;
    case 0xDF: DCP(abs_x); break;
    case 0xDB: DCP(abs_y); break;
    case 0xC3: DCP(idx_ind_x); break;
    case 0xD3: DCP(ind_idx_y); break;
    case 0xE7: ISC(zp); break;
    case 0xF7: ISC(zp_x); break;
    case 0xEF: ISC(abs); break;
    case 0xFF: ISC(abs_x); break;
    case 0xFB: ISC(abs_y); break;
    case 0xE3: ISC(idx_ind_x); break;
    case 0xF3: ISC(ind_idx_y); break;
    case 0x07: SLO(zp); break;
    case 0x17: SLO(zp_x); break;
    case 0x0F: SLO(abs); break;
    case 0x1F: SLO(abs_x); break;
    case 0x1B: SLO(abs_y); break;
    case 0x03: SLO(idx_ind_x); break;
    case 0x13: SLO(ind_idx_y); break;
    case 0x27: RLA(zp); break;
    case 0x37: RLA(zp_x); break;
    case 0x2F: RLA(abs); break;
    case 0x3F: RLA(abs_x); break;
    case 0x3B: RLA(abs_y); break;
    case 0x23: RLA(idx_ind_x); break;
    case 0x33: RLA(ind_idx_y); break;
    case 0x67: RRA(zp); break;
    case 0x77: RRA(zp_x); break;
    case 0x6F: RRA(abs); break;
    case 0x7F: RRA(abs_x); break;
    case 0x7B: RRA(abs_y); break;
    case 0x63: RRA(idx_ind_x); break;
    case 0x73: RRA(ind_idx_y); break;
    case 0x47: SRE(zp); break;
    case 0x57: SRE(zp_x); break;
    case 0x4F: SRE(abs); break;
    case 0x5F: SRE(abs_x); break;
    case 0x5B: SRE(abs_y); break;
    case 0x43: SRE(idx_ind_x); break;
    case 0x53: SRE(ind_idx_y); break;
    case 0x4B: ALR(); break;
    case 0x6B: ARR(); break;
    case 0x0B:
    case 0x2B: ANC(); break;
    case 0x8B: ANE(); break;
    case 0x9F: SHA(abs_y); break;
    case 0x93: SHA(ind_idx_y); break;
    case 0x9B: TAS(); break;
    case 0x9E: SHX(); break;
    case 0x9C: SHY(); break;
    case 0xBB: LAS(); break;
    /* 1-byte NOPs */
    case 0x1A:
    case 0x3A:
    case 0x5A:
    case 0x7A:
    case 0xDA:
    case 0xFA: cycles += 2; break;
    /* 2-byte NOPs */
    case 0x80:
    case 0x82:
    case 0x89:
    case 0xC2:
    case 0xE2:
        PC++;
        cycles += 2;
        break;
    case 0x04:
    case 0x44:
    case 0x64:
        PC++;
        cycles += 3;
        break;
    case 0x14:
    case 0x34:
    case 0x54:
    case 0x74:
    case 0xD4:
    case 0xF4:
        PC++;
        cycles += 4;
        break;
    /* 3-byte NOPs */
    case 0x0C:
        PC += 2;
        cycles += 4;
        break;
    /* Variable cycle count */
    case 0x1C:
    case 0x3C:
    case 0x5C:
    case 0x7C:
    case 0xDC:
    case 0xFC: NOP_absx(); break;
    case 0x02:
    case 0x12:
    case 0x22:
    case 0x32:
    case 0x42:
    case 0x52:
    case 0x62:
    case 0x72:
    case 0x92:
    case 0xB2:
    case 0xD2:
    case 0xF2: JAM(opcode); throw NES::JAM(); break;
    default:
        throw NES::InvalidOpcode();
    }

    if (NMI) {
        std::cerr << "Handling NMI" << std::endl;
        interrupt(i_nmi);
    }
    if (IRQ && !P.I) {
        std::cerr << "Handling IRQ" << std::endl;
        interrupt(i_irq);
    }

    return cycles > initial_cyc
               ? cycles - initial_cyc
               : std::numeric_limits<uint32_t>::max() - initial_cyc + cycles;
}

void CPU::interrupt(NES::Interrupt type) {
    if (type != i_reset) {
        PH((uint8_t)(PC >> 8));
        PH((uint8_t)PC);
        PH(type == i_brk ? P.status | 0x10 : P.status);
    } else {
        P.status |= 0x04;
        // write(0x4015, 0x0);  // All channels disabled
    }

    P.I = true;

    switch (type) {
    case i_nmi: PC = read16(0xFFFA); break;
    case i_reset: PC = read16(0xFFFC); break;
    case i_irq:
    case i_brk:
    default: PC = read16(0xFFFE); break;
    }

    if (type == i_nmi)
        NMI = false;
    else if (type == i_irq)
        IRQ = false;

    cycles += 7;
}

uint8_t CPU::read(uint16_t addr) {
    switch (dma) {
    case DMA_OAM:
    case DMA_PCM: handle_dma();
    default: return bus->read(addr);
    }
}

uint16_t CPU::read16(uint16_t addr, bool zp) {
    // If we know this is a zero-page addr, wrap the most-significant bit
    // around zero-page bounds
    uint16_t h_addr = zp ? ((addr + 1) % 0x100) : (addr + 1);
    uint8_t l_data = read(addr);
    uint8_t h_data = read(h_addr);
    return (h_data << 8) | l_data;
}

void CPU::write(uint16_t addr, uint8_t value) { bus->write(addr, value); }

void CPU::schedule_dma_oam(uint8_t page) {
    std::cerr << "CPU::schedule_dma_oam page: " << page << std::endl;
    dma = DMA_OAM;
    dma_page = page;
}

void CPU::schedule_nmi() { NMI = true; }

void CPU::handle_dma() {
    if (dma == DMA_PCM) {
        throw std::runtime_error("PCM DMA unimplemented");
    } else if (dma == DMA_OAM) {
        // TODO: PCM DMA can interrupt OAM DMA
        if (cycles & 0x1) {
            cycles++;
        }
        uint8_t i = 0;
        do {
            uint8_t data = bus->read((dma_page << 8) | i);
            cycles++;
            bus->write(0x2004, data);
            cycles++;
            i++;
        } while (i != 0);
        dma = DMA_Clear;
    }
}

uint8_t CPU::get_operand(AddressingMode mode) {
    return read(operand_addr(mode));
}

bool CPU::is_same_page(uint16_t addr, uint16_t addr2) {
    return (addr & 0xFF00) == (addr2 & 0xFF00);
}

bool CPU::idx_abs_crossing_cycle(uint8_t opcode) {
    uint8_t incr_ops[] = {
        0x7D, 0x79, 0x3D, 0x39, 0xDD, 0xD9, 0x5D, 0x59, 0xBD, 0xB9,
        0xBE, 0xBC, 0x1D, 0x19, 0xFD, 0xF9, 0xBF, 0x1C, 0x3C, 0x5C,
        0x7C, 0xDC, 0xFC, 0xC9, 0xBF, 0xB3, 0xBD, 0xB9, 0xB1,
    };
    size_t size = sizeof(incr_ops) / sizeof(uint8_t);
    return std::find(incr_ops, incr_ops + size, opcode) != (incr_ops + size);
}

uint16_t CPU::operand_addr(AddressingMode mode) {
    uint16_t addr = 0x0; 
    uint8_t addr_h, addr_l = 0x0;
    uint8_t i = 0x0;
    switch (mode) {
    case abs:
        addr = read16(PC);
        PC += 2;
        break;
    case abs_x:
        addr_l = read(PC);
        addr_h = read(PC+1);
        addr = ((addr_h << 8) | addr_l) + X;
        if (!is_same_page(addr-X, addr))
            read((addr_h << 8) | (uint8_t)(addr_l+X));
        else
            read(addr);
        PC += 2;
        if (idx_abs_crossing_cycle(opcode) && !is_same_page(addr - X, addr))
            cycles++;
        break;
    case abs_y:
        addr_l = read(PC);
        addr_h = read(PC+1);
        addr = ((addr_h << 8) | addr_l) + Y;
        if (!is_same_page(addr-Y, addr))
            read((addr_h << 8) | (uint8_t)(addr_l+Y));
        else
            read(addr);
        PC += 2;
        if (idx_abs_crossing_cycle(opcode) && !is_same_page(addr - Y, addr))
            cycles++;
        break;
    case imm:
        addr = PC;
        PC++;
        break;
    case zp:
        addr = read(PC);
        PC++;
        break;
    case zp_x:
        i = read(PC);
        read(i);
        addr = (i + X) % 0x100;
        PC++;
        break;
    case zp_y:
        i = read(PC);
        read(i);
        addr = (i + Y) % 0x100;
        PC++;
        break;
    case idx_ind_x:
        i = read(PC);
        read(i);
        addr_l = read((i+X) % 0x100);
        addr_h = read((i+X+1) % 0x100);
        addr = (addr_h << 8) | addr_l;
        PC++;
        break;
    case ind_idx_y:
        i = read(PC);
        addr_l = read(i);
        addr_h = read((uint16_t)(uint8_t)(i+1));
        addr = ((addr_h << 8) | addr_l) + Y;
        if (!is_same_page(addr - Y, addr)) 
            read((addr_h << 8) | (uint8_t)(addr_l + Y));
        else 
            read(addr);
        PC++;
        if (idx_abs_crossing_cycle(opcode) && !is_same_page(addr - Y, addr))
            cycles++;
        break;
    case ind:
    default:
        std::cerr << "Invalid addressing mode: " << std::hex << int(mode)
                  << std::endl;
    }
    return addr;
}

// Auxiliary

uint8_t CPU::rot_l(uint8_t value) {
    bool last_C = P.C;
    P.C = bool(value & 0x80);
    uint8_t output = value << 1 | last_C;
    set_NZ(output);
    return output;
}

uint8_t CPU::rot_r(uint8_t value) {
    bool last_C = P.C;
    P.C = bool(value & 1);
    uint8_t output = last_C << 7 | value >> 1;
    set_NZ(output);
    return output;
}

uint8_t CPU::shift_l(uint8_t value) {
    P.C = bool(value & 0x80);
    uint8_t output = (uint8_t)(value << 1);
    set_NZ(output);
    return output;
}

uint8_t CPU::shift_r(uint8_t value) {
    P.C = bool(uint8_t(value << 7));
    uint8_t output = (uint8_t)(value >> 1);
    set_NZ(output);
    return output;
}

void CPU::do_ADC(uint8_t operand) {
    // ADC/SBC implementation:
    // https://stackoverflow.com/questions/29193303/6502-emulation-proper-way-to-implement-adc-and-sbc
    // Overflow on signed arithmetic:
    // http://www.6502.org/tutorials/vflag.html
    // In decimal mode treat operands as binary-coded decimals.
    // TODO: Not sure if additional handling is needed for decimals?
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

void CPU::set_NZ(uint8_t value) {
    P.Z = value == 0;
    P.N = (bool)(value & 0x80);
}

void CPU::BRK() {
    read(PC++);
    interrupt(i_brk);
}

// Branch instructions

void CPU::branch_rel() {
    uint8_t op = get_operand(imm);
    uint8_t pc_h = uint8_t((PC & 0xFF00) >> 8);
    uint8_t pc_l = uint8_t(PC & 0xFF);
    bool page_crossing = uint16_t(pc_l) + op >= 0x100;
    pc_l += op;
    PC = (PC & 0xFF00) | (uint16_t)pc_l;
    if ((op & 0x80) && !page_crossing) {
        PC -= 0x100;
    } else if (!(op & 0x80) && page_crossing) {
        PC += 0x100;
    }
    if (((PC & 0xFF00) >> 8) != pc_h) {
        cycles += 2;
    } else {
        cycles++;
    }
}

#define branch_rel_if(expr) \
    {                       \
        cycles += 2;        \
        if (expr)           \
            branch_rel();   \
        else                \
            PC++;           \
    }

void CPU::BPL() { branch_rel_if(!P.N) }
void CPU::BMI() { branch_rel_if(P.N) }
void CPU::BVC() { branch_rel_if(!P.V) }
void CPU::BVS() { branch_rel_if(P.V) }
void CPU::BCC() { branch_rel_if(!P.C) }
void CPU::BCS() { branch_rel_if(P.C) }
void CPU::BNE() { branch_rel_if(!P.Z) }
void CPU::BEQ() { branch_rel_if(P.Z) }

// Control transfer

void CPU::JMP(AddressingMode mode) {
    switch (mode) {
    case abs:
        PC = read16(PC);
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
        l_addr = read16(PC);
        h_addr = (l_addr % 0x100 == 0xFF) ? (uint16_t)(l_addr - l_addr % 0x100)
                                          : (uint16_t)(l_addr + 1);
        l_addr = read(l_addr);
        h_addr = read(h_addr);
        PC = (uint16_t)h_addr << 8 | l_addr;
        cycles += 5;
        break;
    default:
        std::cerr << "Invalid addressing mode for JMP: " << mode << std::endl;
    }
}

void CPU::JSR() {
    // JSR return address should be the last byte of the 3-byte JSR instr.
    uint8_t addr_l, addr_h = 0x0;
    uint16_t return_addr = (uint16_t)(PC + 1);

    addr_l = read(PC);

    read((uint16_t)(0x100 + S));
    write((uint16_t)(0x100 + S), (uint8_t)(return_addr >> 8));
    S--;
    write((uint16_t)(0x100 + S), (uint8_t)return_addr);
    S--;
    
    addr_h = read(PC+1);

    PC = ((uint16_t)addr_h << 8) | addr_l;
    cycles += 6;
}

void CPU::RTS() {
    uint8_t l_addr, h_addr;
    read(PC);
    read((uint16_t)(0x100 + S));
    S++;
    l_addr = read((uint16_t)(0x100 + S));
    S++;
    h_addr = read((uint16_t)(0x100 + S));
    read(h_addr << 8 | l_addr);
    PC = (h_addr << 8 | l_addr) + 0x1;
    cycles += 6;
}

void CPU::RTI() {
    uint8_t l_addr, h_addr;
    read(PC);
    read((uint16_t)(0x100 + S));
    S++;
    uint8_t newp = read((uint16_t)(0x100 + S));
    P.N = bool(newp & 0x80);
    P.V = bool(newp & 0x40);
    P.D = bool(newp & 0x8);
    P.I = bool(newp & 0x4);
    P.Z = bool(newp & 0x2);
    P.C = bool(newp & 0x1);
    S++;
    l_addr = read((uint16_t)(0x100 + S));
    S++;
    h_addr = read((uint16_t)(0x100 + S));
    PC = (h_addr << 8 | l_addr);
    cycles += 6;
}

// Arithmetic / logical

void CPU::ADC(AddressingMode mode) {
    uint16_t op_addr = operand_addr(mode);
    do_ADC(read(op_addr));
    switch (mode) {
    case imm: cycles += 2; break;
    case zp: cycles += 3; break;
    case zp_x: cycles += 4; break;
    case abs: cycles += 4; break;
    case abs_x: cycles += 4; break;
    case abs_y: cycles += 4; break;
    case idx_ind_x: cycles += 6; break;
    case ind_idx_y: cycles += 5; break;
    default: std::cerr << "Invalid addressing mode for ADC." << std::endl;
    }
}

void CPU::AND(AddressingMode mode) {
    A &= get_operand(mode);
    set_NZ(A);
    switch (mode) {
    case imm: cycles += 2; break;
    case zp: cycles += 3; break;
    case zp_x: cycles += 4; break;
    case abs: cycles += 4; break;
    case abs_x: cycles += 4; break;
    case abs_y: cycles += 4; break;
    case idx_ind_x: cycles += 6; break;
    case ind_idx_y: cycles += 5; break;
    default: std::cerr << "Invalid addressing mode for AND." << std::endl;
    }
}

void CPU::ASL_A() {
    A = shift_l(A);
    cycles += 2;
}

void CPU::ASL(AddressingMode mode) {
    uint16_t addr = operand_addr(mode);
    uint8_t op = read(addr);
    write(addr, op);
    write(addr, shift_l(op));
    switch (mode) {
    case zp: cycles += 5; break;
    case zp_x: cycles += 6; break;
    case abs: cycles += 6; break;
    case abs_x: cycles += 7; break;
    default: std::cerr << "Invalid addressing mode for ASL." << std::endl;
    }
}

void CPU::BIT(AddressingMode mode) {
    uint8_t operand = get_operand(mode);
    P.Z = (A & operand) == 0;
    P.V = (bool)((operand >> 6) & 1);
    P.N = (bool)((operand >> 7) & 1);
    if (mode == zp)
        cycles += 3;
    else if (mode == abs)
        cycles += 4;
}

void CPU::CMP(AddressingMode mode) { CP(A, mode); }

void CPU::CP(const uint8_t &reg, AddressingMode mode) {
    uint8_t operand = get_operand(mode);
    P.Z = reg == operand;
    P.C = reg >= operand;
    P.N = (bool)((reg - operand) & 0x80);
    switch (mode) {
    case imm: cycles += 2; break;
    case zp: cycles += 3; break;
    case zp_x: cycles += 4; break;
    case abs: cycles += 4; break;
    case abs_x: cycles += 4; break;
    case abs_y: cycles += 4; break;
    case idx_ind_x: cycles += 6; break;
    case ind_idx_y: cycles += 5; break;
    default: std::cerr << "Invalid addressing mode for CPx." << std::endl;
    }
}

void CPU::DEC(AddressingMode mode) {
    uint16_t op_addr = operand_addr(mode);
    uint8_t op = read(op_addr);
    uint8_t result = op - 1;
    write(op_addr, op);
    write(op_addr, result);
    set_NZ(result);
    switch (mode) {
    case zp: cycles += 5; break;
    case zp_x: cycles += 6; break;
    case abs: cycles += 6; break;
    case abs_x: cycles += 7; break;
    default: std::cerr << "Invalid addressing mode for DEC." << std::endl;
    }
}

void CPU::EOR(AddressingMode mode) {
    uint8_t operand = get_operand(mode);
    A ^= operand;
    set_NZ(A);
    switch (mode) {
    case imm: cycles += 2; break;
    case zp: cycles += 3; break;
    case zp_x: cycles += 4; break;
    case abs: cycles += 4; break;
    case abs_x: cycles += 4; break;
    case abs_y: cycles += 4; break;
    case idx_ind_x: cycles += 6; break;
    case ind_idx_y: cycles += 5; break;
    default: std::cerr << "Invalid addressing mode for EOR." << std::endl;
    }
}

#define set_status_flag(flag, value) \
    {                                \
        P.flag = value;              \
        cycles += 2;                 \
    }

void CPU::CLC() set_status_flag(C, false);
void CPU::SEC() set_status_flag(C, true);
void CPU::CLI() set_status_flag(I, false);
void CPU::SEI() set_status_flag(I, true);
void CPU::CLV() set_status_flag(V, false);
void CPU::CLD() set_status_flag(D, false);
void CPU::SED() set_status_flag(D, true);

void CPU::LSR_A() {
    A = shift_r(A);
    cycles += 2;
}

void CPU::LSR(AddressingMode mode) {
    uint16_t addr = operand_addr(mode);
    uint8_t op = read(addr);
    uint8_t result = shift_r(op);
    write(addr, op);
    write(addr, result);

    switch (mode) {
    case zp: cycles += 5; break;
    case zp_x: cycles += 6; break;
    case abs: cycles += 6; break;
    case abs_x: cycles += 7; break;
    default: std::cerr << "Invalid addressing mode for LSR." << std::endl;
    }
}

void CPU::ORA(AddressingMode mode) {
    uint8_t operand = get_operand(mode);
    A |= operand;
    set_NZ(A);
    switch (mode) {
    case imm: cycles += 2; break;
    case zp: cycles += 3; break;
    case zp_x: cycles += 4; break;
    case abs: cycles += 4; break;
    case abs_x: cycles += 4; break;
    case abs_y: cycles += 4; break;
    case idx_ind_x: cycles += 6; break;
    case ind_idx_y: cycles += 5; break;
    default: std::cerr << "Invalid addressing mode for CPx." << std::endl;
    }
}

void CPU::ROL_A() {
    A = rot_l(A);
    cycles += 2;
}

void CPU::ROL(AddressingMode mode) {
    uint16_t addr = operand_addr(mode);
    uint8_t op = read(addr);
    write(addr, op);
    write(addr, rot_l(op));
    switch (mode) {
    case zp: cycles += 5; break;
    case zp_x: cycles += 6; break;
    case abs: cycles += 6; break;
    case abs_x: cycles += 7; break;
    default: std::cerr << "Invalid addressing mode for ROL." << std::endl;
    }
}

void CPU::ROR_A() {
    A = rot_r(A);
    cycles += 2;
}

void CPU::ROR(AddressingMode mode) {
    uint16_t addr = operand_addr(mode);
    uint8_t op = read(addr);
    write(addr, op);
    write(addr, rot_r(op));
    switch (mode) {
    case zp: cycles += 5; break;
    case zp_x: cycles += 6; break;
    case abs: cycles += 6; break;
    case abs_x: cycles += 7; break;
    default: std::cerr << "Invalid addressing mode for ROR." << std::endl;
    }
}

void CPU::SBC(AddressingMode mode) {
    uint16_t op_addr = operand_addr(mode);
    do_ADC(~read(op_addr));
    switch (mode) {
    case imm: cycles += 2; break;
    case zp: cycles += 3; break;
    case zp_x: cycles += 4; break;
    case abs: cycles += 4; break;
    case abs_x: cycles += 4; break;
    case abs_y: cycles += 4; break;
    case idx_ind_x: cycles += 6; break;
    case ind_idx_y: cycles += 5; break;
    default: std::cerr << "Invalid addressing mode for SBC." << std::endl;
    }
}

void CPU::USBC() { SBC(imm); }

// Load / store

void CPU::LD(uint8_t &reg, AddressingMode mode) {
    uint8_t operand = get_operand(mode);
    reg = operand;
    set_NZ(operand);
    switch (mode) {
    case imm: cycles += 2; break;
    case zp: cycles += 3; break;
    case zp_x: cycles += 4; break;
    case zp_y: cycles += 4; break;
    case abs: cycles += 4; break;
    case abs_x: cycles += 4; break;
    case abs_y: cycles += 4; break;
    case idx_ind_x: cycles += 6; break;
    case ind_idx_y: cycles += 5; break;
    default: std::cerr << "Invalid addressing mode for LDx." << std::endl;
    }
}

void CPU::ST(uint8_t reg, AddressingMode mode) {
    uint16_t op_addr = operand_addr(mode);
    write(op_addr, reg);
    switch (mode) {
    case zp: cycles += 3; break;
    case zp_x: cycles += 4; break;
    case zp_y: cycles += 4; break;
    case abs: cycles += 4; break;
    case abs_x:
    case abs_y: cycles += 5; break;
    case idx_ind_x: cycles += 6; break;
    case ind_idx_y: cycles += 6; break;
    default: std::cerr << "Invalid addressing mode for STx." << std::endl;
    }
}

void CPU::INC(AddressingMode mode) {
    uint16_t addr = operand_addr(mode);
    uint8_t op = read(addr);
    auto result = (uint8_t)(op + 1);
    write(addr, op);
    write(addr, result);
    set_NZ(result);
    switch (mode) {
    case zp: cycles += 5; break;
    case zp_x: cycles += 6; break;
    case abs: cycles += 6; break;
    case abs_x: cycles += 7; break;
    default: std::cerr << "Invalid addressing mode for INC." << std::endl;
    }
}

// Register

void CPU::T(uint8_t &reg_from, uint8_t &reg_to) {
    reg_to = reg_from;
    cycles += 2;
    if (std::addressof(reg_from) == std::addressof(X) &&
        std::addressof(reg_to) == std::addressof(S))
        return;
    set_NZ(reg_from);
}

void CPU::DE(uint8_t &reg) {
    reg--;
    set_NZ(reg);
    cycles += 2;
}

void CPU::IN(uint8_t &reg) {
    reg++;
    set_NZ(reg);
    cycles += 2;
}

// Stack

void CPU::PH(uint8_t value) {
    read(PC);
    write((uint16_t)(0x100 + S), value);
    S--;
    cycles += 3;
}

void CPU::PH(const StatusRegister &p) {
    read(PC);
    write((uint16_t)(0x100 + S), (uint8_t)(p.status | 0x10));
    S--;
    cycles += 3;
}

void CPU::PL(uint8_t &reg_to) {
    read(PC);
    read(0x100 + S);
    S++;
    uint8_t operand = read((uint16_t)(0x100 + S));
    reg_to = operand;
    if (&reg_to != &P.status) set_NZ(operand);
    cycles += 4;
}

void CPU::PL(StatusRegister &p) {
    read(PC);
    read(0x100 + S);
    S++;
    uint8_t newp = read((uint16_t)(0x100 + S));
    p.N = bool(newp & 0x80);
    p.V = bool(newp & 0x40);
    p.D = bool(newp & 0x8);
    p.I = bool(newp & 0x4);
    p.Z = bool(newp & 0x2);
    p.C = bool(newp & 0x1);

    cycles += 4;
}

void CPU::NOP_absx() {
    get_operand(abs_x);
    cycles += 4;
}

void CPU::LAX(AddressingMode mode) {
    uint8_t operand = get_operand(mode);
    A = operand;
    X = operand;
    set_NZ(operand);
    switch (mode) {
    case zp: cycles += 3; break;
    case zp_y: cycles += 4; break;
    case abs: cycles += 4; break;
    case abs_y: cycles += 4; break;
    case idx_ind_x: cycles += 6; break;
    case ind_idx_y: cycles += 5; break;
    default: std::cerr << "Invalid addressing mode for LAX." << std::endl;
    }
}

void CPU::LXA() {
    uint8_t op = get_operand(imm);
    A = (A | 0xEE) & op;
    X = A;
    set_NZ(A);
    cycles += 2;
}

void CPU::SAX(AddressingMode mode) {
    uint16_t op_addr = operand_addr(mode);
    write(op_addr, A & X);
    switch (mode) {
    case zp: cycles += 3; break;
    case zp_y: cycles += 4; break;
    case abs: cycles += 4; break;
    case idx_ind_x: cycles += 6; break;
    default: std::cerr << "Invalid addressing mode for SAX." << std::endl;
    }
}

void CPU::SBX() { // AXS
    // Another op with weird behaviour. Disabled tests
    uint8_t op = get_operand(imm);
    X = (A & X) - op;

    P.Z = X == 0;
    P.C = X >= 0;
    P.N = (bool)(X & 0x80);

    cycles += 2;
}

void CPU::DCP(AddressingMode mode) {
    uint16_t op_addr = operand_addr(mode);
    uint8_t op = read(op_addr);
    uint8_t result = op - 1;
    write(op_addr, op);
    write(op_addr, result);

    P.Z = A == result;
    P.C = A >= result;
    P.N = bool((A - result) & 0x80);

    switch (mode) {
    case zp: cycles += 5; break;
    case zp_x: cycles += 6; break;
    case abs: cycles += 6; break;
    case abs_x: cycles += 7; break;
    case abs_y: cycles += 7; break;
    case idx_ind_x: cycles += 8; break;
    case ind_idx_y: cycles += 8; break;
    default: std::cerr << "Invalid addressing mode for DCP." << std::endl;
    }
}

void CPU::ISC(AddressingMode mode) {
    // INC
    uint16_t op_addr = operand_addr(mode);
    uint8_t op = read(op_addr);
    uint8_t result = op + 1;
    write(op_addr, op);
    write(op_addr, result);
    set_NZ(result);

    // SBC
    do_ADC(~result);

    switch (mode) {
    case zp: cycles += 5; break;
    case zp_x: cycles += 6; break;
    case abs: cycles += 6; break;
    case abs_x: cycles += 7; break;
    case abs_y: cycles += 7; break;
    case idx_ind_x: cycles += 8; break;
    case ind_idx_y: cycles += 8; break;
    default:
        std::cerr << "Invalid addressing mode for ISC (ISB, INS)." << std::endl;
    }
}

void CPU::SLO(AddressingMode mode) {
    // ASL
    uint16_t addr = operand_addr(mode);
    uint8_t op = read(addr);
    uint8_t result = shift_l(op);
    write(addr, op);
    write(addr, result);

    // ORA
    A |= result;
    set_NZ(A);

    switch (mode) {
    case zp: cycles += 5; break;
    case zp_x: cycles += 6; break;
    case abs: cycles += 6; break;
    case abs_x: cycles += 7; break;
    case abs_y: cycles += 7; break;
    case idx_ind_x: cycles += 8; break;
    case ind_idx_y: cycles += 8; break;
    default: std::cerr << "Invalid addressing mode for SLO/ASO." << std::endl;
    }
}

void CPU::RLA(AddressingMode mode) {
    // ROL
    uint16_t addr = operand_addr(mode);
    uint8_t op = read(addr);
    uint8_t result = rot_l(op);
    write(addr, op);
    write(addr, result);

    // AND
    A &= result;
    set_NZ(A);

    switch (mode) {
    case zp: cycles += 5; break;
    case zp_x: cycles += 6; break;
    case abs: cycles += 6; break;
    case abs_x: cycles += 7; break;
    case abs_y: cycles += 7; break;
    case idx_ind_x: cycles += 8; break;
    case ind_idx_y: cycles += 8; break;
    default: std::cerr << "Invalid addressing mode for SLO/ASO." << std::endl;
    }
}

void CPU::SRE(AddressingMode mode) {
    // LSR
    uint16_t addr = operand_addr(mode);
    uint8_t op = read(addr);
    uint8_t result = shift_r(op);
    write(addr, op);
    write(addr, result);

    // EOR
    A ^= result;
    set_NZ(A);

    switch (mode) {
    case zp: cycles += 5; break;
    case zp_x: cycles += 6; break;
    case abs: cycles += 6; break;
    case abs_x: cycles += 7; break;
    case abs_y: cycles += 7; break;
    case idx_ind_x: cycles += 8; break;
    case ind_idx_y: cycles += 8; break;
    default: std::cerr << "Invalid addressing mode for SRE." << std::endl;
    }
}

void CPU::RRA(AddressingMode mode) {
    // ROR
    uint16_t addr = operand_addr(mode);
    uint8_t op = read(addr);
    uint8_t result = rot_r(op);
    write(addr, op);
    write(addr, result);

    // ADC
    do_ADC(result);

    switch (mode) {
    case zp: cycles += 5; break;
    case zp_x: cycles += 6; break;
    case abs: cycles += 6; break;
    case abs_x: cycles += 7; break;
    case abs_y: cycles += 7; break;
    case idx_ind_x: cycles += 8; break;
    case ind_idx_y: cycles += 8; break;
    default: std::cerr << "Invalid addressing mode for RRA." << std::endl;
    }
}

void CPU::ALR() {
    A &= get_operand(imm);
    P.C = (bool)(A & 0x1);
    A = shift_r(A);
    set_NZ(A);
    cycles += 2;
}

void CPU::ARR() { 
    uint8_t op = get_operand(imm);
    A &= op;
    A = P.C << 7 | A >> 1;
    set_NZ(A);
    P.C = bool((A >> 6) & 0x1);
    P.V = bool(P.C ^ ((A >> 5) & 0x1));
    cycles += 2;
}

void CPU::ANC() {
    AND(imm);
    P.C = (bool)(A & 0x80);
}

void CPU::ANE() { // XAA
    uint8_t op = get_operand(imm);
    A = (A | 0xEE) & X & op;
    set_NZ(A);
}

void CPU::SHA(AddressingMode mode) {
    // TODO: Missing behaviour, disabled tests for this. Attempt to model
    // unstability below in TAS 
    // unstable: sometimes 'AND (H+1)' is dropped, page boundary crossings may 
    // not work (with the high-byte of the value used as the high-byte of the 
    // address)
    uint16_t addr = operand_addr(mode);
    uint8_t result = A & X & ((addr >> 8) + 1);
    write(addr, result); 
    switch (mode) {
    case abs_y:
        cycles += 5; break;
    case ind_idx_y:
        cycles += 6; break;
    default: std::cerr << "Invalid addressing mode for SHA." << std::endl;
    }
}

void CPU::TAS() { // XAS / SHS
    // TODO: Disabled tests, not sure if I can satisfy them
    // unstable: sometimes 'AND (H+1)' is dropped, page boundary crossings may 
    // not work (with the high-byte of the value used as the high-byte of the 
    // address)  
    // (And sometimes the addr gets incremented by 1 additionally)

    // abs_y
    uint16_t addr;
    uint8_t addr_l, addr_h;
    uint8_t result;
    addr_l = read(PC);
    addr_h = read(PC+1);
    addr = ((addr_h << 8) | addr_l) + Y;
    if (!is_same_page(addr-Y, addr))
        read((addr_h << 8) | (uint8_t)(addr_l+Y));
    else
        read(addr);
    PC += 2;

    result = A & X & ((addr >> 8) + 1);
    S = A & X;

    if (!is_same_page(addr-Y, addr))
        addr = (result << 8) | (uint8_t)addr;

    write(addr, result);
    cycles += 5;
}

void CPU::SHX() {
    // Unstable as above
    // abs_y
    uint16_t addr;
    uint8_t addr_l, addr_h;
    uint8_t result;
    addr_l = read(PC);
    addr_h = read(PC+1);
    addr = ((addr_h << 8) | addr_l) + Y;
    if (!is_same_page(addr-Y, addr))
        read((addr_h << 8) | (uint8_t)(addr_l+Y));
    else
        read(addr);
    PC += 2;

    result = X & ((addr >> 8) + 1);

    if (!is_same_page(addr-Y, addr))
        addr = (result << 8) | (uint8_t)addr;

    write(addr, result);
    cycles += 5;
}

void CPU::SHY() {
    // Unstable as above
    // abs_x
    uint16_t addr;
    uint8_t addr_l, addr_h;
    uint8_t result;
    addr_l = read(PC);
    addr_h = read(PC+1);
    addr = ((addr_h << 8) | addr_l) + X;
    if (!is_same_page(addr-X, addr))
        read((addr_h << 8) | (uint8_t)(addr_l+X));
    else
        read(addr);
    PC += 2;

    result = Y & ((addr >> 8) + 1);

    if (!is_same_page(addr-X, addr))
        addr = (result << 8) | (uint8_t)addr;

    write(addr, result);
    cycles += 5;
}

void CPU::LAS() {
    //uint16_t addr = operand_addr(abs_y);
    //uint8_t operand = read(addr);

    // abs_y
    uint8_t addr_l = read(PC);
    uint8_t addr_h = read(PC+1);
    uint16_t addr = ((addr_h << 8) | addr_l) + Y;
    PC += 2;
    if (idx_abs_crossing_cycle(opcode) && !is_same_page(addr - Y, addr))
        cycles++;
    uint8_t operand = read(addr);
    S &= operand;
    A = X = S;
    set_NZ(A);
    cycles += 4;
}

void CPU::JAM(uint8_t opcode) {
    read(PC);
    read(0xffff); 
    read(0xfffe); 
    read(0xfffe); 
    read(0xffff); 
    read(0xffff); 
    read(0xffff); 
    read(0xffff); 
    read(0xffff); 
    read(0xffff); 
}
