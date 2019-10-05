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
		/// Initializes an NROM (iNES Mapper 0) Cartridge Mapper
		/// instance.
		/// \param cartridge Cartridge to use.
		explicit NROM(Cartridge &cartridge);
		
		/// Reads a byte of memory at the provided address.
		uint8_t read(uint16_t addr) final;
		
		/// Writes a byte of memory to the provided address.
		void write(uint16_t addr, uint8_t val) final;
	};
	
	/// MMC1 Control Register.
	union MMC1CR
	{
		struct
		{
			uint8_t M : 2; 	///< Mirroring type:
					///< 0b00 - 1-screen mirroring nametable 0.
					///< 0b01 - 1-screen mirroring nametable 1.
					///< 0b10 - Vertical mirroring.
					///< 0b11 - Horizontal mirroring.
			bool H : 1;	///< PRG ROM swap bank:
					///< 0 - Low bank fixed, high bank swappable.
					///< 1 - Low bank swappable, high bank fixed.
			bool F : 1;	///< PRG swappable bank size:
					///< 0 - 32K
					///< 1 - 16K
			bool C : 1;	///< CHR bank size:
					///< 0 - Single 8K bank in CHR space.
					///< 0 - Two 4K banks in CHR space.
		};
		uint8_t value;		///< Integer representation of the
					///< register.
	};
	
	class MMC1 : public NES::iNESv1::Mapper::Base
	{
	public:
		/// Initializes an MMC1 (iNES Mapper 1) Cartridge Mapper
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
		MMC1CR reg0;		///< Main control register 0.
		uint8_t[4] regs;	///< MMC1 5-bit control/bank registers.
					///< 0: Main control register
					///< 1: CHR ROM low bank register
					///< 2: CHR ROM high bank register
					///< 3: PRG ROM bank register
		
		/// Resets the shift register.
		void reset_shift_reg();
		
		/// Returns the number of the accessed register based on the
		/// address used.
		int reg_number(uint16_t addr);
		
		/// Sets a register using contents from the shift register.
		/// \param reg_number Register number to select.
		void set_register(int reg_number);
	};
	
	class UnimplementedType {};
	class InvalidAddress {};
}
}
}

#endif //INC_2A03_CARTRIDGE_H
