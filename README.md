# 2a03

NES emulator written in C++. Implements elements of a NES system: Ricoh 2A03 (MOS 6502 w/o decimal mode) CPU, 2C02 NTSC PPU with RGB output, memory bus, DMA and cartridge interface in software. 

CPU state matches with Nintendulator nestest.nes run up to the part where it starts writing to the APU/PPU at around 26k cycles, where it starts breaking as the APU does not exist yet. Since there is no real timing implemented, it just goes through instructions as fast as it can for now. 

### Feature list

In progress:
- PPU - Registers, MMIO ports, VRAM, OAM, secondary OAM
- OAM DMA
- SDL2 window output for the PPU

CPU:
- Official opcodes
- Partial unofficial opcodes
- Cycle counting
- Addressing modes
- Memory bus
- Interrupts

Cartridges:
- iNESv1 cartridge model
- NROM(001), MMC1 mapper

Utils:
- CPU debug logger
