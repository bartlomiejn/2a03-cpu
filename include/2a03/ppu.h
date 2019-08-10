#ifndef INC_2A03_PPU_H
#define INC_2A03_PPU_H

#include <cstdint>

namespace NES
{
	/// NES PPU 2C02 emulator.
	/// http://wiki.nesdev.com/w/index.php/PPU_registers
	class PPU
	{
	public:
		uint8_t PPUCTRL;	/// PPU Controller (W)
		uint8_t PPUMASK; 	/// PPU Mask (W)
		uint8_t PPUSTATUS; 	/// PPU Status (R)
		uint8_t OAMADDR;	/// OAM Address (W)
		uint8_t OAMDATA;	/// OAM Data (R/W)
		uint8_t PPUSCROLL;	/// Scroll (W x2)
		uint8_t PPUADDR;	/// Address (W x2)
		uint8_t PPUDATA;	/// Data (R/W)
		uint8_t OAMDMA;		/// OAM DMA (W)
	};
}

#endif //INC_2A03_PPU_H
