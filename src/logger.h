#ifndef INC_2A03_LOGGER_H
#define INC_2A03_LOGGER_H

#include <bus.h>
#include <cpu.h>
#include <ppu.h>

#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace NES {
class SystemLogger {
   public:
    /// Instantiates a SystemLogger instance which logs the state of
    /// the provided CPU.
    /// \param cpu CPU instance to use.
    /// \param ppu PPU instance to use.
    /// \param bus BUS instance to get opcode/operand data from.
    /// Should be the same instance as the one used by the CPU.
    SystemLogger(NES::CPU &cpu, NES::PPU &ppu, NES::MemoryBus &bus);

    /// Two 8-bit reads on the bus with behaviour same as CPU
    uint16_t bus_read16(uint16_t addr, bool zp);

    /// If set writes every line on each log() call to this stream
    std::optional<std::reference_wrapper<std::ostream>> instr_ostream;
    /// If set specifies the output log filename
    std::optional<std::string> log_filename;

    /// Logs a line with CPU state.
    std::string log();

    /// Dumps the accumulated log to a file.
    void save();

   protected:
    NES::CPU &cpu;                  ///< CPU whose state is logged.
    NES::PPU &ppu;                  ///< PPU whose state is logged.
    NES::MemoryBus &bus;            ///< Bus whose devices are logged.
    std::vector<std::string> logs;  ///< Contains logs of CPU state
                                    ///< on each `log` call.

   private:
    /// Decodes an opcode into a readable string form.
    static std::string decode(uint8_t opcode);

    /// Returns the addressing mode for an opcode, if it's
    /// applicable.
    static std::optional<NES::AddressingMode> addr_mode_for_op(uint8_t opcode);

    /// Returns a templated string for provided mode.
    /// \param addr_mode Addressing mode to provide a template for.
    /// \return Templated string for provided mode with optional
    /// operand marked as `{{OPERAND}}` and optional target marked
    /// as `{{TARGET}}`.
    static std::string templ_for_mode(NES::AddressingMode addr_mode,
                                      uint8_t opcode);

    /// Returns the operand length in bytes for provided mode.
    static uint8_t operand_len(NES::AddressingMode addr_mode);

    /// If there is a target for a specified mode, returns
    /// the amount of bytes to be printed.
    static uint8_t target_len(NES::AddressingMode addr_mode, uint8_t opcode);

    /// Retrieve target value for specified addressing mode.
    uint16_t target_value(NES::AddressingMode addr_mode);

    /// Returns true if provided `opcode` is part of the official
    /// instruction set.
    static bool is_opcode_legal(uint8_t opcode);
};
}  // namespace NES

#endif  // INC_2A03_LOGGER_H
