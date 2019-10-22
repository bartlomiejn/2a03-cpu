#ifndef INC_2A03_PPU_H
#define INC_2A03_PPU_H

#include <cstdint>
#include <2a03/utils/bitfield.h>

namespace NES
{
	bitfield_union(Register0, uint8_t value,
		bool x_scroll_ntable : 1; 	///< X scroll nametable selection.
		bool y_scroll_ntable : 1; 	///< Y scroll nametable selection.
		bool incr_ppu_addr : 1; 	///< Increment PPU address by 1/32 (0/1) on access to port 7.
		bool obj_pat_table_sel : 1;	///< Object pattern table selection.
		bool play_pat_table_sel : 1; 	///< Playfield pattern table selection.
		bool scanline_obj : 1;		///< 8/16 (0/1) scanline objects.
		bool ext_bus_dir : 1;		///< EXT bus direction in/out (0/1).
		bool vbl_disable : 1;		///< VBL disable (when 0).
	);
	
	bitfield_union(Register1, uint8_t value,
		bool disable_comp_color : 1; 	///< Disable composite colorburst (when 1).
		bool left_scr_col_play_clip : 1; ///< Y scroll nametable selection.
		bool left_scr_col_obj_clip : 1; ///< Increment PPU address by 1/32 (0/1) on access to port 7.
		bool play_display : 1;		///< Enable playfield display (when 1).
		bool obj_display : 1;		///< Enable object display (when 1).
		bool r : 1;
		bool g : 1;
		bool b : 1;
	);
	
	bitfield_union(Register2, uint8_t value,
		bool reserved : 5;
		bool obj_on_scanline : 1;	///< More than 8 objects on a single scanline have been detected in the last frame.
		bool obj_play_collide : 1;	///< A primary object pixel has collided with a playfield pixel in the last frame.
		bool vblank : 1;		///< VBlank flag.
	);
	
	/// Ricoh 2C03 PPU emulator.
	class PPU
	{
	public:
		/// Creates a PPU instance.
		PPU();
		
		Register0 r0;
		Register1 r1;
		Register2 r2;
		uint8_t r3; 	///< Internal object attribute memory index
				///< pointer. (64 attributes, 32 bits each, byte
				///< access). Stored value post-increments on
				///< access to port 4.
		uint8_t r4;	///< Object attribute memory location indexed by
				///< port 3, then increments port 3.
		uint8_t r5;	///< Scroll offset port.
		uint8_t r6;	///< PPU address port to access with port 7.
		uint8_t r7;	///< PPU memory read/write port.
	private:
	};
}

#endif //INC_2A03_PPU_H
