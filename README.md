# 2a03

NES emulator written in C++. 
 
Currently: The CPU is implemented and works 100% correctly for official opcodes. 

To build: `make binary`

To run: `make run`

To debug: `make debug CXX_DEBUG={Default: gdb}` 

## Feature list

### v0.2

In progress:
- Cycle counting
- Unofficial instructions

Done:
- Official opcode correctness

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