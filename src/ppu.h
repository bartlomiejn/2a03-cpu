#ifndef INC_2A03_PPU_H
#define INC_2A03_PPU_H

#include <bitfield.h>
#include <mapper.h>
#include <palette.h>
#include <gui.h>

#include <array>
#include <cstdint>
#include <functional>
#include <stdexcept>

namespace NES {

/// 2000 PPUCTRL write register
bitfield_union(
    PPUCTRL, uint8_t value,
    uint8_t nt_xy_select : 2;  ///< Base nametable addr 0=$2000, 1=$2400,
                               ///< 2=$2800, 3=$2C00
    bool v_incr : 1;           ///< 1: Vertical v increment (+32 instead of +1)
    bool spr_pt_addr : 1;      ///< 0: $0000 1: $1000, ignored in 8x16 mode
    bool bg_pt_addr : 1;       ///< 0: $0000 1: $1000
    bool spr_size : 1;    ///< 0: 8x8 pixels, 1: 8x16 pixels (see PPU OAM byte
                          ///< 1)
    bool ppu_master : 1;  ///< EXT bus direction 0: input, 1: output
    bool vbl_nmi : 1;     ///< Generate NMI at start of vertical blanking
                          ///< interval (1: on).
);

/// 2001 PPUMASK write register
bitfield_union(
    PPUMASK, uint8_t value,
    bool grayscale : 1;          ///< Disable colorburst: 1: Grayscale 0: Color
    bool bg_show_left_8px : 1;   ///< 1: Show background in leftmost 8
                                 ///< pixels of the screen
    bool spr_show_left_8px : 1;  ///< 1: Show sprites in leftmost 8
                                 ///< pixels of the screen
    bool bg_show : 1;            ///< 1: Show background/playfield
    bool spr_show : 1;           ///< 1: Show sprites/objects
    bool r : 1;                  ///< Emphasize R
    bool g : 1;                  ///< Emphasize G
    bool b : 1;                  ///< Emphasize B
);

/// 2002 PPUSTATUS r/o register
bitfield_union(
    PPUSTATUS, uint8_t value,
    bool ppu_open_bus : 5;  ///< Returns stale PPU bus contents.
    bool spr_overflow : 1;  ///< 1: When more than 8 sprites are in scanline
    bool spr0_hit : 1;      ///< 1: When a nonzero pixel of sprite 0 overlaps a
                            ///< nonzero bg pixel, used for raster timing
    bool vblank : 1;        ///< 0: Not in vblank 1: in vblank
);

/// Internal PPU VRAM register
#pragma pack(push, 1)
union PPUVramAddr {
    struct __attribute__((packed)) {
        uint8_t sc_x : 5;       ///< Coarse X scroll
        uint8_t sc_y : 5;       ///< Coarse Y scroll
        bool nt_h : 1;          ///< Nametable select horizontal
        bool nt_v : 1;          ///< Nametable select vertical
        uint8_t sc_fine_y : 3;  ///< Fine Y scroll
    };
    struct {
        uint8_t l : 8;  ///< 8-bit VRAM address low bits
        uint8_t h : 7;  ///< 7-bit VRAM address high bits
    };
    struct {
        uint16_t addr : 15;
    };
};
#pragma pack(pop)

/// Object attributes model for sprites
struct OA {
    bitfield_union(Attributes, uint8_t value,
                   bool palette : 2;   ///< Palette select
                   bool RESERVED : 3;  ///< Reserved
                   bool obj_pri : 1;   ///< Object priority: 0: Higher than BG
                   bool flip_h : 1;    ///< Flip sprite pixels horizontally
                   bool flip_v : 1;    ///< Flip sprite pixels vertically
    );
    uint8_t y;     ///< Scanline Y (top-most) coordinate
    uint8_t tile;  ///< Tile index number = PPUCTRL base + tile number.
                   ///< With PPUCTRL bit 5 set (8x16), bit 0 selects the pattern
                   ///< table
    Attributes attr;  ///< Attributes byte
    uint8_t x;        ///< Scanline X (left-side) coordinate
};

static const size_t vram_sz = 0x800;    ///< PPU VRAM size
static const size_t oam_sz = 0x100;     ///< PPU OAM size
static const size_t oam_sec_sz = 0x20;  ///< Secondary OAM memory size
static const size_t pram_sz = 0x20;     ///< Palette RAM size

static const size_t ntsc_x = 341;  ///< NTSC pixel count (341 PPU clock cycles
                                   ///< per scanline)
static const size_t ntsc_y = 262;  ///< NTSC scanline count

static const size_t ntsc_fb_x = 256;  ///< NTSC framebuffer X size
static const size_t ntsc_fb_y = 240;  ///< NTSC framebuffer Y size
static const size_t ntsc_fb_sz = ntsc_fb_x * ntsc_fb_y;

/// Ricoh 2C02 NTSC PPU emulator
class PPU {
   public:
    iNESv1::Mapper::Base *mapper;  ///< Cartridge mapper
    GFX::GUI &gui;                 ///< Draws the actual frames
    NES::Palette pal;              ///< Palette file

    // Internal memory
    std::array<uint8_t, vram_sz> vram;        ///< PPU VRAM
    std::array<uint8_t, oam_sz> oam;          ///< PPU OAM
    std::array<uint8_t, oam_sec_sz> oam_sec;  ///< Secondary OAM
    std::array<uint8_t, pram_sz> pram;        ///< Palette RAM

    // Internal PPU registers
    PPUVramAddr v;  ///< 15-bit Current VRAM addr
    PPUVramAddr t;  ///< 15-bit Temporary VRAM addr / Top left onscreen tile
    struct {
        uint8_t fine : 3;
    } x;     ///< 3-bit Fine X scroll register
    bool w;  ///< 1-bit internal pointer flip-flop

    struct {
        uint16_t addr : 14;
    } bus;  // Internal buffer

    uint8_t nt;  // Tile idx for pattern lookup
    uint8_t at;  // Paltete info for a region of tiles (4x4 tiles = 32x32 px)
    uint8_t at_latch_l;  // AT palette bit L latch
    uint8_t at_latch_h;  // AT palette bit H latch
    uint8_t bg_latch_l;  // NT L byte latch
    uint8_t bg_latch_h;  // NT H byte latch
    uint16_t bg_l_shift;
    uint16_t bg_h_shift;
    uint16_t at_l_shift;  // AT palette L bit shift register
    uint16_t at_h_shift;  // AT palette H bit shift register

    uint8_t cpu_bus;

    // CPU memory mapped registers
    PPUCTRL ppuctrl;      ///< PPUCTRL, write access $2000
    PPUMASK ppumask;      ///< PPUMASK, write access $2001
    PPUSTATUS ppustatus;  ///< PPUSTATUS, read access $2002
    // $2003 OAMADDR
    // $2004 OAMDATA
    // $2005 PPUSCROLL
    // $2006 PPUADDR
    // $2007 PPUDATA

    uint8_t oamaddr;      ///< 8-bit OAM address register
    uint8_t oamdata;      ///< 8-bit OAM data buffer
    uint8_t oam_sec_addr;
    bool oam_overflow;
    bool oam_sec_overflow;

    /// Sprite output unit latches, loaded during sprite fetch (cycles 257-320)
    struct SpriteOut {
        uint8_t pat_l;  ///< Pattern table low byte
        uint8_t pat_h;  ///< Pattern table high byte
        uint8_t attr;   ///< Attribute byte (palette, priority, flips)
        uint8_t x;      ///< X position counter
    };
    std::array<SpriteOut, 8> spr_out;
    bool spr0_in_range;  ///< Sprite 0 is in secondary OAM this scanline

    uint8_t ppudata_buf;  ///< 8-bit PPUADDR read buffer

    // Output
    alignas(16) std::array<uint32_t, ntsc_fb_sz> fb;      ///< Framebuffer
    alignas(16) std::array<uint32_t, ntsc_fb_sz> fb_sec;  ///< Secondary
    bool fb_prim = true;

    std::function<void()> on_nmi_vblank;  ///< Issues a VBlank NMI

    uint16_t scan_x;      ///< Pixel
    uint16_t scan_y;      ///< Scanline
    uint16_t scan_x_end;  ///< Pixels count
    uint16_t scan_y_end;  ///< Scanline count
    bool scan_short;      ///< Short scanline (340 ticks instead of 341)

    bool headless = false;      ///< Skip GUI rendering for profiling
    uint64_t frame_count = 0;   ///< Completed frame counter

    PPU(GFX::GUI &_gui, NES::Palette _pal);

    /// Powers up the PPU
    void power();

    /// Executes the PPU logic.
    /// \value cycles PPU cycles to execute
    void execute(uint16_t cycles);

    /// Writes value @ addr from CPU bus
    void cpu_write(uint16_t addr, uint8_t value);

    /// Reads value @ addr from CPU bus
    /// \param addr Address to read
    /// \param passive Don't trigger additional behaviour, just read
    uint8_t cpu_read(uint16_t addr, bool passive=false);

   protected:
    /// Write value to addr
    void write(uint16_t addr, uint8_t value);

    /// Reads value from addr
    uint8_t read(uint16_t addr);

    /// Draws a pixel for the current cycle
    void draw();

    // Sprite-related logic
    void oam_sec_clear();
    void sprite_eval();
    void sprite_fetch();
};

}  // namespace NES

#endif  // INC_2A03_PPU_H
