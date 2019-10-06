#ifndef INC_2A03_CARTRIDGE_H
#define INC_2A03_CARTRIDGE_H

#include <2a03/cart/ines.h>

namespace NES
{
namespace iNESv1
{
namespace Mapper
{
	class Base;
	
	enum Type
	{
		type_NROM,
		type_MMC1
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
	
	class MMC1 : public NES::iNESv1::Mapper::Base
	{
	public:
		enum MMC1Register
		{ reg_main_control, reg_l_chrrom, reg_h_chrrom, reg_prg_bank };
		enum PRGBankSwap { swap_h_prg_bank, swap_l_prg_bank };
		enum PRGBankSize { size_32k, size_16k };
		enum CHRBankSize { size_8k, size_4k };
		
		/// Initializes an MMC1 (iNES Mapper 1) Cartridge Mapper
		/// instance.
		/// \param cartridge Cartridge to use.
		explicit MMC1(Cartridge &cartridge);
		
		/// Reads a byte of memory at the provided address.
		uint8_t read(uint16_t addr) final;
		
		/// Writes a byte of memory to the provided address.
		void write(uint16_t addr, uint8_t val) final;
	private:
		// Shift register contents
		uint8_t shift_reg; 		///< Shift register.
		uint8_t shift_count; 		///< Shift counter.
		
		// Main Control Register
		PRGBankSwap prg_bank_swap; 	///< Decides which PRG Bank is swappable.
		PRGBankSize prg_bank_sz; 	///< PRG bank size.
		CHRBankSize chr_bank_sz;	///< CHR bank size.
		
		// PRG ROM Bank Register
		uint8_t prg_bank; 		///< PRG ROM bank number.
		bool wram_enable;		///< Decides if WRAM is enabled.
		
		/// Resets the shift register.
		void reset_shift_reg();
		
		/// Returns the number of the accessed register based on the
		/// address used.
		int reg_number(uint16_t addr);
		
		/// Sets a register using contents from the shift register.
		/// \param reg_number Register number to select.
		void set_reg(int reg_number);
		
		/// Sets the main control register using contents from the
		/// shift register.
		void set_main_ctrl_reg(uint8_t value)
		
		/// Sets the PRG ROM bank register using contents from the
		/// shift register.
		void set_prg_bank_reg(uint8_t value)
	};
	
	class UnimplementedType {};
	class InvalidAddress {};
}
}
}

#endif //INC_2A03_CARTRIDGE_H
