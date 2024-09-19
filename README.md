# 2a03

NES emulator written in C++. Implements a Ricoh 2A03 CPU and a 2C02(2C07) PPU behavior in software. 

CPU state matches with Nintendulator nestest.nes run up to the part where it starts writing to the APU/PPU at around 26k cycles. Since there is no real timing yet, it just goes through instructions as fast as it can. 

### Feature list

In progress:
- PPU / OAM DMA
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
