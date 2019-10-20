# 2a03

NES emulator written in C++. 

tl;dr version: The CPU works 100% correctly for official opcodes... and only CPU. 

## Build instructions

`make binary CXX={Your C++ compiler}`

I've put my compiler temporarily as the default value which most definitely won't work with your setup, so make sure to fill it in if you want to build it. 

## Feature list

#### v0.2 In progress
- Cycle counting
- Unofficial instructions

#### v0.2 Upcoming
- Official instruction correctness

#### v0.1
- Instruction cycle
- Addressing modes
- Memory bus
- Official opcodes
- Interrupts
- iNESv1 Cartridge model with loading
- NROM, MMC1 mapper
- Binary execution
- Debug logger