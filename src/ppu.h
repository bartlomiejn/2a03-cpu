#ifndef INC_2A03_PPU_H
#define INC_2A03_PPU_H

#include <utils/bitfield.h>

#include <cstdint>

namespace NES {
bitfield_union(
    Register0, uint8_t value,
    bool x_scroll_ntable : 1;  ///< X scroll nametable selection.
    bool y_scroll_ntable : 1;  ///< Y scroll nametable selection.
    bool incr_ppu_addr : 1;  ///< Increment PPU address by 1/32 (0/1) on access
                             ///< to port 7.
    bool obj_pat_table_sel : 1;    ///< Object pattern table selection.
    bool playf_pat_table_sel : 1;  ///< Playfield pattern table selection.
    bool scanline_obj : 1;         ///< 8/16 (0/1) scanline objects.
    bool ext_bus_dir : 1;          ///< EXT bus direction in/out (0/1).
    bool vbl_disable : 1;          ///< VBL disable (when 0).
);

bitfield_union(
    Register1, uint8_t value,
    bool disable_comp_color : 1;  ///< Disable composite colorburst (when 1).
    bool left_scr_col_playf_clip : 1;  ///< Y scroll nametable selection.
    bool left_scr_col_obj_clip : 1;  ///< Increment PPU address by 1/32 (0/1) on
                                     ///< access to port 7.
    bool playf_display : 1;          ///< Enable playfield display (when 1).
    bool obj_display : 1;            ///< Enable object display (when 1).
    bool r : 1; bool g : 1; bool b : 1;);

bitfield_union(
    Register2, uint8_t value,
    bool RESERVED : 5;
    bool obj_on_scanline : 1;   ///< More than 8 objects on a single scanline
                                ///< have been detected in the last frame.
    bool obj_play_collide : 1;  ///< A primary object pixel has collided with a
                                ///< playfield pixel in the last frame.
    bool vblank : 1;            ///< VBlank flag.
);

struct ObjectAttribute {
    bitfield_union(
        Byte2, uint8_t value,
        bool pal_sel_l : 1;  ///< Palette select low bit.
        bool pal_sel_h : 1;  ///< Palette select high bit.
        bool RESERVED : 3; bool
            obj_pri : 1;  ///< Object priority: > playfield / < playfield (0/1).
        bool bit_reverse : 1;  ///< Apply bit reversal to fetched object pattern
                               ///< table data.
        bool inv_scan_addr : 1;  ///< Invert the 3/4-bit (8/16 scanlines/object
                                 ///< mode) scanline address used to access an
                                 ///< object tile.
    );

    uint8_t
        coord;  ///< Scanline coordinate minus one of object's top pixel row.
    uint8_t tile_idx;  ///< Tile index number. Bit 0 controls pattern table
                       ///< selection when r0.5 == 1.
    Byte2 b2;
    uint8_t scan_pix_coord;  ///< Scanline pixel coordinate of most left-hand
                             ///< side of object.
};

/// Ricoh 2C03 PPU emulator.
class PPU {
   public:
    /// Creates a PPU instance.
    PPU();

    Register0 r0;
    Register1 r1;
    Register2 r2;
    uint8_t r3;  ///< Internal object attribute memory index
                 ///< pointer. (64 attributes, 32 bits each, byte
                 ///< access). Stored value post-increments on
                 ///< access to port 4.
    uint8_t r4;  ///< Object attribute memory location indexed by
                 ///< port 3, then increments port 3.
    uint8_t r5;  ///< Scroll offset port.
    uint8_t r6;  ///< PPU address port to access with port 7.
    uint8_t r7;  ///< PPU memory read/write port.
   private:
};
}  // namespace NES

#endif  // INC_2A03_PPU_H
