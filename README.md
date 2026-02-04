# 2a03

NES emulator written in C++. Implements elements of a NES system: Ricoh 2A03 (MOS 6502 w/o decimal mode) CPU, 2C02 NTSC PPU, NROM cartridge mapper. Includes SDL2 based graphics and a debug view for inspecting PPU/pattern table state.

CPU currently passes nestest in automated mode and nes6502 single instruction CPU tests aside from a couple of unstable instructions with weird behaviour - generally CPU is more or less correct, including cycle counting. PPU currently displays a very interesting and stable blob of grey and black blocks, but its slowly getting there.

