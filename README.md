# 2a03

NES emulator written in C++. 

tl;dr version: The CPU works 100% correctly for official opcodes. 

To build: `make binary`

To run: `make run`

To debug: `make debug CC_DB={Default: gdb}` 

## Feature list

#### v0.2 In progress
- Cycle counting
- Unofficial instructions

#### v0.2 Upcoming
- Official instruction correctness

#### v0.1
Release date: 20.10.2019
- Instruction cycle
- Addressing modes
- Memory bus
- Official opcodes
- Interrupts
- iNESv1 Cartridge model with loading
- NROM, MMC1 mapper
- Binary execution
- Debug logger