#ifndef INC_2A03_BUS_H
#define INC_2A03_BUS_H

#include <cstdint>
#include <array>
#include <optional>
#include <2a03/cartridge/mapper.h>

static const int internal_ram_sz = 0x800; ///< NES Internal RAM size.

namespace NES
{
	class MemoryBus
	{
	public:
		std::array<uint8_t, internal_ram_sz> ram; ///< Internal RAM
		std::optional<iNESv1::Mapper> mapper; ///< Cartridge mapper, if it's inserted.
		
		/// Initializes the memory bus.
		MemoryBus();
		
		/// Reads 8 bits of memory at the provided address.
		/// \param addr Address to read from.
		/// \return Byte that has been read.
		uint8_t read(uint16_t addr);
		
		// TODO: is_zp is redundant, we can infer this from the address
		/// Reads 16 bits of memory at the provided address.
		/// \param addr Address to read from
		/// \param is_zp If it's a zero-page address, wrap the most
		/// -significant byte around zero-page.
		/// \return 2 bytes that have been read.
		uint16_t read16(uint16_t addr, bool is_zp = false);
		
		/// Writes a value to the provided address.
		/// \param addr Address to write the value to.
		/// \param val Value to write.
		void write(uint16_t addr, uint8_t val);
	};
	
	class MissingCartridge {};
}

#endif //INC_2A03_BUS_H
