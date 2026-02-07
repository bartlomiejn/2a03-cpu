# 2a03

NES emulator written in C++. Implements elements of a NES system: Ricoh 2A03 (MOS 6502 w/o decimal mode) CPU and a 2C02 NTSC PPU. Includes SDL2 based graphics, a debug view for inspecting emulation state and quite a lot of logging.

Can run NROM games, ones that are known to work:
- Super Mario Bros. 1
- Donkey Kong

Controller keys are hardcoded: WSAD - Dpad, I - Start, O - Select, K - Button A, L - Button B.
