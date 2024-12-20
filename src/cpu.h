#ifndef INC_2A03_CPU_H
#define INC_2A03_CPU_H

#include <bitfield.h>

#include <cstdint>

namespace NES {

class MemoryBusIntf;

/// Addressing mode for an operation.
enum AddressingMode {
    rel,        ///< Relative (branch instructions)
    abs,        ///< Absolute.
    abs_x,      ///< Absolute indexed with X.
    abs_y,      ///< Absolute indexed with Y.
    imm,        ///< Immediate.
    zp,         ///< Zero Page.
    zp_x,       ///< Zero Page indexed with X.
    zp_y,       ///< Zero Page indexed with Y.
    idx_ind_x,  ///< Indexed indirect with X.
    ind_idx_y,  ///< Indirect indexed with Y.
    ind         ///< Indirect.
};

/// Interrupt type.
enum Interrupt { i_irq, i_nmi, i_reset, i_brk };

/// 2A03 status register bitfield union representation.
bitfield_union(StatusRegister, uint8_t status,
               bool C : 1;     ///< (bit0) Carry. Set if the result value is greater
                               ///< than 0xFF.
               bool Z : 1;     ///< Zero. Set if result value is 0.
               bool I : 1;     ///< Interrupt disable
               bool D : 1;     ///< Decimal mode
               uint8_t B : 2;  ///< Has no effect on CPU, but certain
                               ///< instructions set it.
               bool V : 1;     ///< Overflow. Used in signed arithmetic ops.
                               ///< Set if the result value is outside of
                               ///< [-128, 127] range, which is a two
                               ///< complements overflow.
               bool N : 1;     ///< Negative. Set using the 7-th bit of a
                               ///< result value.
);

/// Ricoh 2A03 CPU emulator. CPU state is invalid until `power` is
/// called.
class CPU {
   public:
    NES::MemoryBusIntf *bus;

    uint8_t A;         ///< Accumulator
    uint8_t X, Y;      ///< Index registers
    uint16_t PC;       ///< Program counter
    uint8_t S;         ///< Stack pointer
    StatusRegister P;  ///< Status register
    bool IRQ;          ///< Interrupt line. Setting to true will trigger an
                       ///< IRQ after next instruction completes.
    bool NMI;  ///< Non-maskable interrupt line. Setting to true will trigger an
               ///< IRQ after next instruction completes.
    uint32_t cycles;  ///< Cycle counter.
    uint8_t opcode; ///< Current opcode.

    CPU(NES::MemoryBusIntf *bus);

    /// Does crossing the page boundary result in an additional cycle for
    /// this opcode
    static bool idx_abs_crossing_cycle(uint8_t opcode);

    /// Returns `true` if addr and addr2 are on the same page.
    static bool is_same_page(uint16_t addr, uint16_t addr2);

    /// Starts the CPU.
    void power();

    /// Resets the CPU.
    void reset();

    /// Executes the next instruction.
    /// \return Cycles executed
    uint16_t execute();

    /// Schedules a DMA transfer
    /// \value page Page to copy
    void schedule_dma_oam(uint8_t page);

    /// Schledules an NMI after the next instruction cycles finish
    void schedule_nmi();

   protected:
    enum DMAState {
        DMA_Clear,
        DMA_OAM,
        DMA_PCM,
    };

    DMAState dma = DMA_Clear;  ///< Is DMA scheduled/in progress
    uint8_t dma_page = 0x0;    ///< Page to transfer

    /// Handle DMA transfer
    void handle_dma();

    /// Attempt to read byte from bus at addr
    uint8_t read(uint16_t addr);

    /// Attempt to read 2 bytes from bus
    /// \param addr Address to read from
    /// \param zp If it's a zero-page addr, wrap the most significant byte
    /// around zero-page
    /// \return Read bytes
    uint16_t read16(uint16_t addr, bool zp = false);

    /// Attempt to write byte to bus at addr
    void write(uint16_t addr, uint8_t value);

    /// Performs an interrupt routine
    void interrupt(Interrupt type);

    // Addressing mode functions
    // http://www.obelisk.me.uk/6502/addressing.html

    /// Returns the address of the parameter based on the addressing
    /// mode and increments PC based on param length.
    /// \param mode Addressing mode.
    /// \return Parameter address.
    uint16_t operand_addr(AddressingMode mode);

    /// Retrieves the current instruction parameter based on
    /// the addressing mode and increments PC based on parameter
    /// length.
    /// \param mode Addressing mode to use.
    /// \return Parameter for current instruction.
    uint8_t get_operand(AddressingMode mode);

    // Auxiliary functions

    /// Rotates left the value once. Carry is shifted into output
    /// bit 0, input bit 7 is shifted into Carry. Sets N, Z, C.
    /// \param value Value to rotate left.
    /// \return Rotated value.
    uint8_t rot_l(uint8_t value);

    /// Rotates right the value once. Carry is shifted into output
    /// bit 7, input bit 0 is shifted into Carry. Sets N, Z, C.
    /// \param value Value to rotate right.
    /// \return Rotated value.
    uint8_t rot_r(uint8_t value);

    /// Shifts left the value once. 0 is shifted into bit 0, bit 7
    /// is shifted into Carry. Sets N, Z, C.
    /// \param value Value to shift left.
    /// \return Shifted value.
    uint8_t shift_l(uint8_t value);

    /// Shifts right the value once. 0 is shifted into bit 7, bit 0
    /// is shifted into Carry. Sets N, Z, C.
    /// \param value Value to shift right.
    /// \return Shifted value.
    uint8_t shift_r(uint8_t value);

    /// Performs ADC with the provided operand.
    void do_ADC(uint8_t operand);

    /// Sets the N and Z flags based on the value provided.
    void set_NZ(uint8_t value);

    // Instructions
    // http://www.6502.org/tutorials/6502opcodes.html - Docs

    void BRK();

    // Branch instructions

    /// Does a relative branch using the opcode's operand.
    void branch_rel();

    /// Branch on plus.
    void BPL();

    /// Branch on minus.
    void BMI();

    /// Branch on overflow clear
    void BVC();

    /// Branch on overflow set
    void BVS();

    /// Branch on carry clear
    void BCC();

    /// Branch on carry set
    void BCS();

    /// Branch on not equal
    void BNE();

    /// Branch on equal
    void BEQ();

    // Control transfer

    /// Transfer program execution.
    /// \param mode Addressing mode to use.
    void JMP(AddressingMode mode);

    /// Jump to subroutine. Pushes the address - 1 of next op on the
    /// stack before transfering control.
    void JSR();

    /// Return from subroutine. Pulls two bytes off the stack (low
    /// byte first) and transfers control to that address.
    void RTS();

    /// Return from interrupt. Retrieves the status word and the PC
    /// value from the stack (first status, then PC).
    void RTI();

    // Arithmetic / logical

    /// Add with carry.
    /// \param mode Addressing mode to use.
    void ADC(AddressingMode mode);

    /// Bitwise AND with accumulator.
    /// \param mode Addressing mode to use.
    void AND(AddressingMode mode);

    /// Arithmetic shift left with accumulator. 0 is shifted into
    /// bit 0 and the original bit 7 is shifted into Carry.
    void ASL_A();

    /// Arithmetic shift left. 0 is shifted into bit 0 and the
    /// original bit 7 is shifted into Carry.
    /// \param mode Addressing mode to use.
    void ASL(AddressingMode mode);

    /// Test if one or more bits are set in a target memory
    /// location. Effectively ANDs with the accumulator which should
    /// contain a mask pattern. Affects N, V, Z.
    /// \param mode Addressing mode to use.
    void BIT(AddressingMode mode);

    /// Compare accumulator. Sets flags as if subtraction had been
    /// carried out. If A >= operand, sets C. N will be set based
    /// on the sign of result and equality of the operand.
    /// \param mode Addressing mode to use.
    void CMP(AddressingMode mode);

    /// Compare register. Sets flags as if subtraction had been
    /// carried out. If A >= operand, sets C. N will be set based
    /// on the sign of result and equality of the operand.
    /// \param reg Register to compare to.
    /// \param mode Addressing mode to use.
    void CP(const uint8_t &reg, AddressingMode mode);

    /// Decrement memory. Affects N, Z.
    /// \param mode Addressing mode to use.
    void DEC(AddressingMode mode);

    /// Bitwise XOR. Affects N, Z.
    void EOR(AddressingMode mode);

    /// Clear carry flag.
    void CLC();

    /// Set carry flag.
    void SEC();

    /// Clear interrupt flag.
    void CLI();

    /// Set interrupt flag.
    void SEI();

    /// Clear overflow flag.
    void CLV();

    /// Clear decimal flag.
    void CLD();

    /// Set decimal flag.
    void SED();

    /// Logical shift right with accumulator. 0 is shifted into bit
    /// 7 and the original bit 0 is shifted into Carry.
    void LSR_A();

    /// Logical shift right. 0 is shifted into bit 7 and the
    /// original bit 0 is shifted into Carry.
    /// \param mode Addressing mode to use.
    void LSR(AddressingMode mode);

    /// Performs bitwise OR with the accumulator.
    /// \param mode Addressing mode to use.
    void ORA(AddressingMode mode);

    /// Rotate left the accumulator.
    void ROL_A();

    /// Rotate left a value at the specified address.
    /// \param mode Addressing mode to use.
    void ROL(AddressingMode mode);

    /// Rotate right the accumulator.
    void ROR_A();

    /// Rotate right a value at the specified address.
    /// \param mode Addressing mode to use.
    void ROR(AddressingMode mode);

    /// Subtract with carry.
    /// \param mode Addressing mode to use.
    void SBC(AddressingMode mode);

    // Load / store

    /// Load register with memory.
    /// \param reg Register address to load memory to.
    /// \param addr_fn Addressing mode to use.
    void LD(uint8_t &reg, AddressingMode mode);

    /// Store register.
    /// \param reg Value to store.
    /// \param mode Addressing mode to use.
    void ST(uint8_t reg, AddressingMode mode);

    /// Increment memory.
    /// \param mode Addressing mode to use.
    void INC(AddressingMode mode);

    // Register

    /// Transfer between registers.
    /// \param reg_from Register to transfer from.
    /// \param reg_to Register to transfer to.
    void T(uint8_t &reg_from, uint8_t &reg_to);

    /// Decrement register.
    void DE(uint8_t &reg);

    /// Increment register.
    void IN(uint8_t &reg);

    // Stack

    /// Push value to stack.
    void PH(uint8_t value, uint8_t do_read = true);

    /// Push P to stack.
    void PH(const StatusRegister &p);

    /// Pull value from stack.
    /// \param reg_to Register to pull the value to.
    void PL(uint8_t &reg_to);

    /// Pull value to status register.
    /// \param p P register to pull the value to.
    void PL(StatusRegister &p);

    // Unofficial "illegal" opcodes

    // NOP with variable cycle count
    void NOP_absx();

    // LDA + LDX == MEM -> A -> X
    void LAX(AddressingMode mode);

    // (A OR CONST) AND oper -> A -> X
    void LXA();

    // A AND X -> MEM
    void SAX(AddressingMode mode);

    // (A AND X - oper) -> X
    void SBX();

    // SBC + NOP, effectively same as SBC(imm)
    void USBC();

    // DEC + CMP
    // MEM - 1 -> MEM
    // A - MEM
    // Decrement operand and compare result to A
    void DCP(AddressingMode mode);

    // INC + SBC
    // MEM + 1 -> MEM
    // A - MEM - C -> A
    void ISC(AddressingMode mode);

    // ASL + ORA
    // M = C <- [76543210] <- 0, A OR MEM -> A
    void SLO(AddressingMode mode);

    // ROL + AND
    // M = C <- [76543210] <- 0, A AND MEM -> A
    void RLA(AddressingMode mode);

    // LSR + EOR
    // MEM = 0 -> [76543210] -> C, A EOR M -> A
    void SRE(AddressingMode mode);

    // ROR + ADC
    // M = C -> [76543210] -> C,
    // A + M + C -> A, C
    void RRA(AddressingMode mode);

    // AND oper + LSR
    void ALR();

    // AND oper + ROR
    void ARR();

    // AND + set C as ASL
    // A AND oper, bit(7) -> C
    void ANC();

    // (A OR CONST/random) AND X AND oper -> A
    void ANE();

    // A AND X AND (H+1) -> M
    void SHA(AddressingMode mode);

    // A AND X -> SP, A AND X AND (H+1) -> M
    void TAS();
    
    // X AND (H+1) -> M
    void SHX();

    // Y AND (H+1) -> M
    void SHY();

    // M AND SP -> A, X, SP
    void LAS();

    // Halt execution
    void JAM(uint8_t opcode);
};

class InvalidOpcode {};
class JAM {};

}  // namespace NES

#endif  // INC_2A03_CPU_H
