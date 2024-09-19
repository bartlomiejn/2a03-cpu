#ifndef INC_2A03_PPU_H
#define INC_2A03_PPU_H

#include <utils/bitfield.h>

#include <array>
#include <cstdint>
#include <stdexcept>

namespace NES {

bitfield_union(
    PPUCTRL, uint8_t value,
    uint8_t ntable_base_addr : 2;  ///< Base nametable addr 0=$2000, 1=$2400,
                                   ///< 2=$2800, 3=$2C00
    bool vram_addr_incr : 1;  ///< Increment VRAM addr per CPU r/w of PPUDATA
    bool spr_pattern_tbl_addr : 1;  ///< 0: $0000 1: $1000, ignored in 8x16 mode
    bool bg_pattern_tbl_addr : 1;   ///< 0: $0000 1: $1000
    bool sprite_size : 1;  ///< 0: 8x8 pixels, 1: 8x16 pixels (see PPU OAM byte
                           ///< 1)
    bool ppu_master : 1;   ///< PPU master/slave select (0: read backdrop from,
                           ///< 1: output color) on EXT pins
    bool vbl_nmi : 1;      ///< Generate NMI at start of vertical blanking
                           ///< interval (1: on).
);

bitfield_union(PPUMASK, uint8_t value,
               bool grayscale : 1;         ///< 1: Grayscale 0: Color
               bool bg_show_left_8px : 1;  ///< 1: Show background in leftmost 8
                                           ///< pixels of the screen
               bool spr_show_left_8px : 1;  ///< 1: Show sprites in leftmost 8
                                            ///< pixels of the screen
               bool bg_show : 1;            ///< 1: Show background
               bool spr_show : 1;           ///< 1: Show sprites
               bool r : 1;                  ///< Emphasize R
               bool g : 1;                  ///< Emphasize G
               bool b : 1;                  ///< Emphasize B
);

bitfield_union(
    PPUSTATUS, uint8_t value,
    bool ppu_open_bus : 5;  ///< Returns stale PPU bus contents.
    bool spr_overflow : 1;  ///< 1: When more than 8 sprites are in scanline
    bool spr0_hit : 1;      ///< 1: When a nonzero pixel of sprite 0 overlaps a
                            ///< nonzero bg pixel, used for raster timing
    bool vblank : 1;        ///< 0: Not in vblank 1: in vblank
);

bitfield_union(PPU_vramaddr, uint16_t value,
               uint8_t scrollx : 5;   ///< Coarse X scroll
               uint8_t scrolly : 5;   ///< Coarse Y scroll
               uint8_t ns : 2;        ///< Nametable select
               uint8_t scrollfy : 3;  ///< Fine Y scroll
);

struct OA {
    bitfield_union(
        Attribute, uint8_t value,
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

    uint8_t y;       ///< Scanline Y (topmost) coordinate
    uint8_t tile;    ///< Tile index number.
    Attribute attr;  ///< Attribute
    uint8_t x;       ///< Scanline X (left-side) coordinate
};

static const int vram_sz = 0x3F1F;  ///< PPU VRAM size
static const int oam_sz = 0xFF;     ///< PPU OAM size
static const int ntsc_x = 341;      ///< NTSC pixel count (341 PPU clock cycles
                                    ///< per scanline)
static const int ntsc_y = 262;      ///< NTSC scanline count

/// Ricoh 2C02/7 PPU emulator
class PPU {
   public:
    std::array<uint8_t, vram_sz> vram;         ///< PPU VRAM
    std::array<uint8_t, oam_sz> oam;           ///< PPU OAM
    std::array<uint32_t, ntsc_x * ntsc_y> fb;  ///< Framebuffer

    uint16_t scan_x = 0;  ///< Pixel
    uint16_t scan_y = 0;  ///< Scanline

    PPU_vramaddr v;  ///< 15-bit Current VRAM address
    PPU_vramaddr t;  ///< 15-bit Temporary VRAM address / Top left onscreen tile
    uint8_t x;       ///< 3-bit Fine X scroll
    bool w;          ///< H/V? First or second write toggle

    PPUCTRL ppuctrl;      ///< PPU control register, write access $2000
    PPUMASK ppumask;      ///< PPU mask register, write access $2001
    PPUSTATUS ppustatus;  ///< PPU status register, read access $2002

    uint16_t oamaddr;  ///< 8-bit port at $2003, $2004 is OAMDATA

    uint8_t ppuscrollx;  ///< PPU scrolling position register $2005,
                         ///< write twice (x, y)
    uint8_t ppuscrolly;  ///< PPU scrolling position register $2005,
                         ///< write twice (x, y)
    bool ppu_x = true;

    uint16_t ppuaddr16 = 0x0;  // 8-bit port at $2006, $2007 is PPUDATA
    bool ppu_h = true;

    PPU();

    /// Writes value @ addr
    void write(uint16_t addr, uint8_t value);

    /// Reads value @ addr
    uint8_t read(uint16_t addr);

   protected:
    /// Handle PPUSCROLL X/Y write
    void write_ppuscroll(uint8_t value);

    /// Handle OAMADDR write and OAMDATA MMIO
    void write_oamaddr(uint8_t value);
    void write_oamdata(uint8_t value);

    /// Handle H/L PPUADDR write and PPUDATA MMIO
    void write_ppuaddr(uint8_t value);
    void write_ppudata(uint8_t value);
};

}  // namespace NES

#endif  // INC_2A03_PPU_H
