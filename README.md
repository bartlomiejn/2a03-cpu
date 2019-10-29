# 2a03

NES emulator written in C++.  

To build: `make binary`

To run: `make run`

To debug: `make debug CXX_DEBUG={Default: gdb}` 

## Feature list

### v0.2

In progress:
- Official opcode correctness
- Cycle counting correctness
- PPU

Done:
- Cycle counting

### v0.1
Release: `20.10.2019`
- Instruction cycle
- Addressing modes
- Memory bus
- Official opcodes
- Interrupts
- iNESv1 Cartridge model with loading
- NROM, MMC1 mapper
- Binary execution
- Debug logger
