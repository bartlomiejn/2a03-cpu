#ifndef INC_2A03_CARTRIDGE_H
#define INC_2A03_CARTRIDGE_H

#include <2a03/cartridge/ines.h>

namespace NES
{
namespace iNESv1
{
namespace Mapper
{
	class Base;
	
	enum Type
	{
		type_NROM = 0,
		type_MMC1 = 1
	};
	
	/// Returns an appropriate mapper type for the cartridge provided.
	/// \param for_cartridge Cartridge to generate a mapper for.
	/// \return Returns a concrete mapper instance based on the ID in the
	/// cartridge.
	Mapper::Base *mapper(NES::iNESv1::Cartridge &for_cartridge);
	
	class Base
	{
	public:
		/// Initializes a Cartridge Mapper instance.
		/// \param cartridge Cartridge to use.
		explicit Base(Cartridge &cartridge) : cartridge(cartridge) {};

		/// Reads a byte of memory at the provided address.
		virtual uint8_t read(uint16_t addr) = 0;
		
		/// Writes a byte of memory to the provided address.
		virtual void write(uint16_t addr, uint8_t val) = 0;
	protected:
		Cartridge &cartridge; ///< Cartridge to map.
	};
	
	class NROM : public Mapper::Base
	{
	public:
		/// Initializes an NROM (Mapper 0) Cartridge Mapper
		/// instance.
		/// \param cartridge Cartridge to use.
		explicit NROM(Cartridge &cartridge);
		
		/// Reads a byte of memory at the provided address.
		uint8_t read(uint16_t addr) final;
		
		/// Writes a byte of memory to the provided address.
		void write(uint16_t addr, uint8_t val) final;
	};
	
	class MMC1 : public Mapper::Base
	{
	public:
		/// Initializes an MMC1 (Mapper 1) Cartridge Mapper
		/// instance.
		/// \param cartridge Cartridge to use.
		explicit MMC1(Cartridge &cartridge);
		
		/// Reads a byte of memory at the provided address.
		uint8_t read(uint16_t addr) final;
		
		/// Writes a byte of memory to the provided address.
		void write(uint16_t addr, uint8_t val) final;
	private:
		uint8_t shift_reg; 	///< Shift register.
		uint8_t shift_count; 	///< Shift counter.
		uint8_t l_prgrom_bank; 	///< Low PRG ROM bank number.
		uint8_t h_prgrom_bank; 	///< High PRG ROM bank number.
		
		/// Resets the shift register.
		void reset_shift_reg();
	};
	
	class UnimplementedType {};
}
}
}

#endif //INC_2A03_CARTRIDGE_H
