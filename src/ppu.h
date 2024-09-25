#ifndef INC_2A03_PPU_H
#define INC_2A03_PPU_H

#include <cart/mapper.h>
#include <palette.h>
#include <utils/bitfield.h>

#include <array>
#include <cstdint>
#include <functional>
#include <stdexcept>

namespace NES {

/// PPUCTRL write register
bitfield_union(
    PPUCTRL, uint8_t value,
    uint8_t nt_xy_select : 2;  ///< Base nametable addr 0=$2000, 1=$2400,
                               ///< 2=$2800, 3=$2C00
    bool vram_addr_incr : 1;   ///< Increment VRAM addr per CPU r/w of PPUDATA
    bool spr_pattern_tbl_addr : 1;  ///< 0: $0000 1: $1000, ignored in 8x16 mode
    bool bg_pattern_tbl_addr : 1;   ///< 0: $0000 1: $1000
    bool spr_size : 1;    ///< 0: 8x8 pixels, 1: 8x16 pixels (see PPU OAM byte
                          ///< 1)
    bool ppu_master : 1;  ///< EXT bus direction 0: input, 1: output
    bool vbl_nmi : 1;     ///< Generate NMI at start of vertical blanking
                          ///< interval (1: on).
);

/// PPUMASK write register
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

/// PPUSTATUS r/o register
bitfield_union(
    PPUSTATUS, uint8_t value,
    bool ppu_open_bus : 5;  ///< Returns stale PPU bus contents.
    bool spr_overflow : 1;  ///< 1: When more than 8 sprites are in scanline
    bool spr0_hit : 1;      ///< 1: When a nonzero pixel of sprite 0 overlaps a
                            ///< nonzero bg pixel, used for raster timing
    bool vblank : 1;        ///< 0: Not in vblank 1: in vblank
);

bitfield_union(
    PPUInternalScrFineX, uint8_t value,
    uint8_t fine_x : 3; ///< Fine X scroll
);

/// Internal PPU VRAM register
bitfield_union(PPUInternalReg, uint16_t value,
               uint8_t sc_x : 5;       ///< Coarse X scroll
               uint8_t sc_y : 5;       ///< Coarse Y scroll
               bool nt_h : 1;          ///< Nametable select horizontal
               bool nt_v : 1;          ///< Nametable select vertical
               uint8_t sc_fine_y : 3;  ///< Fine Y scroll
);

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

/// CHR tile addressing struct
bitfield_union(CHRTile, uint8_t value,
               bool T : 1;     ///< Table selector 0 or 1
               uint8_t N : 8;  ///< Tile ID [0x0-0xFF]
               bool P : 1;     ///< Plane 0 or 1
               uint8_t y : 3;  ///< Row # or Y
);

static const size_t vram_sz = 0x3F20;   ///< PPU VRAM size
static const size_t oam_sz = 0x100;     ///< PPU OAM size
static const size_t oam_sec_sz = 0x20;  ///< Secondary OAM memory size

static const size_t ntsc_x = 341;  ///< NTSC pixel count (341 PPU clock cycles
                                   ///< per scanline)
static const size_t ntsc_y = 262;  ///< NTSC scanline count

static const size_t ntsc_fb_x = 256;  ///< NTSC framebuffer X size
static const size_t ntsc_fb_y = 240;  ///< NTSC framebuffer Y size

/// Ricoh 2C02 NTSC PPU emulator
class PPU {
   public:
    // CHR memory
    iNESv1::Mapper::Base *mapper;  ///< Cartridge mapper

    // Internal memory
    std::array<uint8_t, vram_sz> vram;        ///< PPU VRAM
    std::array<uint8_t, oam_sz> oam;          ///< PPU OAM
    std::array<uint8_t, oam_sec_sz> oam_sec;  ///< Secondary OAM

    // Internal PPU registers
    PPUInternalReg v;  ///< 15-bit Current VRAM addr
    PPUInternalReg t;  ///< 15-bit Temporary VRAM addr / Top left onscreen tile
    PPUInternalScrFineX x;  ///< 3-bit Fine X scroll register
    bool w;     ///< 1-bit internal flip-flop 
                ///< first or second write toggle register to PPUSCROLL and 
                ///< PPUADDR

    // CPU memory mapped registers
    PPUCTRL ppuctrl;      ///< PPU control register, write access $2000
    PPUMASK ppumask;      ///< PPU mask register, write access $2001
    PPUSTATUS ppustatus;  ///< PPU status register, read access $2002

    uint16_t oamaddr;  ///< 8-bit port at $2003, $2004 is OAMDATA

    uint8_t ppuscrollx;  ///< PPU scrolling position register $2005,
                         ///< write twice (x, y)
    uint8_t ppuscrolly;  ///< PPU scrolling position register $2005,
                         ///< write twice (x, y)

    uint16_t ppuaddr16 = 0x0;  // 8-bit port at $2006, $2007 is PPUDATA

    // Output
    NES::Palette pal;                                ///< Palette file
    std::array<uint32_t, ntsc_fb_x * ntsc_fb_y> fb;  ///< Framebuffer

    std::function<void(std::array<uint32_t, ntsc_fb_x * ntsc_fb_y> &)>
        frame_ready;

    std::function<void()> nmi_vblank;  ///< Issues a VBlank NMI

    uint16_t scan_x = 0;  ///< Pixel
    uint16_t scan_y = 0;  ///< Scanline

    PPU(NES::Palette _pal);

    /// Powers up the PPU
    void power();

    /// Executes the PPU logic.
    /// \value cycles PPU cycles to execute
    void execute(uint8_t cycles);

    /// Rendering scanline logic
    void render();

    /// Nametable fetch
    void vram_fetch_nt();

    /// Attribute table fetch
    void vram_fetch_at();

    /// Fetch pattern low bits
    void vram_fetch_bg_l();

    /// Fetch pattern high bits
    void vram_fetch_bg_h();

    /// Fetch sprite low bits
    void vram_fetch_spr_l();

    /// Fetch sprite high bits
    void vram_fetch_spr_h();

    /// Write value @ addr to CHR memory
    void chr_write(uint16_t addr, uint8_t value);

    /// Reads value @ addr from CHR memory
    uint8_t chr_read(uint16_t addr);

    /// Writes value @ addr from CPU
    void cpu_write(uint16_t addr, uint8_t value);

    /// Reads value @ addr from CPU
    uint8_t cpu_read(uint16_t addr);

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
