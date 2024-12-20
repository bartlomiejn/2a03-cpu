# 2a03

NES emulator written in C++. Implements elements of a NES system: Ricoh 2A03 (MOS 6502 w/o decimal mode) CPU, 2C02 NTSC PPU with RGB output, memory bus, cartridge mappers/interface in software. 

CPU state matches with Nintendulator nestest.nes run log, 65x02/nes6502 single-instruction tests all pass aside from unstable unofficial opcodes which are modelled only partially. Since there is no real timing implemented it just goes through instructions as fast as it can with CPU and PPU sync also remaining an open topic for now.

### Feature list

In progress:
- PPU + SDL2 window output

CPU:
- Official opcodes
- Unofficial opcodes
- Cycle counting
- Addressing modes
- Memory bus
- Interrupts
- OAM DMA

Cartridges:
- iNESv1 cartridge model
- NROM

Utils:
- CPU/PPU debug logger
- Testing utilities
