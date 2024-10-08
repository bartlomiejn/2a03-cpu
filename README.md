# 2a03

NES emulator written in C++. Implements elements of a NES system: Ricoh 2A03 (MOS 6502 w/o decimal mode) CPU, 2C02 NTSC PPU with RGB output, memory bus, cartridge mappers/interface in software. 

CPU state matches with Nintendulator nestest.nes run up to the part it starts writing to the APU/PPU at around 26k cycles, where it starts breaking as the APU/PPU does not exist/work yet. Since there is no real timing implemented it just goes through instructions as fast as it can with CPU and PPU sync also remaining an open topic for now.

### Feature list

In progress:
- PPU + SDL2 window output
- MMC1

CPU:
- Official opcodes
- Partial unofficial opcodes
- Cycle counting
- Addressing modes
- Memory bus
- Interrupts

Cartridges:
- iNESv1 cartridge model
- NROM

Utils:
- CPU debug logger
