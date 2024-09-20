#include <ppu.h>

// https://fceux.com/web/help/PPU.html

// PPU memory map
//
// Mapped by cartridge:
// $0000-$0FFF - CHR (Pattern) table 0
// $1000-$1FFF - CHR (Pattern) table 1
// $2000-$23FF - Nametable 0
// $2400-$27FF - Nametable 1
// $2800-$2BFF - Nametable 2
// $2C00-$2FFF - Nametable 3
//
// PPU internal:
// $3F00-$3F1F - Palette RAM indices
// Mirrored to $3FFF

using namespace NES;

PPU::PPU(NES::Palette _pal) : pal(std::move(_pal)) {}

void PPU::power() {
    v.value = 0;
    t.value = 0;
    x = 0;
    w = false;
    ppuctrl.value = 0x0;
    ppumask.value = 0x0;
    ppustatus.value = 0x0;
    ppuscrollx = 0x0;
    ppuscrolly = 0x0;
    oamaddr = 0x0;
    // ppudata read buffer = $00
    scan_y = 0;
    scan_x = 0;
}

void PPU::execute(uint8_t cycles) {
    // NTSC
    // If rendering off, each frame is 341*262 / 3 CPU clocks long
    // Scanline (341 pixels) is 113+(2/3) CPU clocks long
    // HBlank (85 pixels) is 28+(1/3) CPU clocks long
    // Frame is 29780.5 CPU clocks long
    // 3 PPU dots per 1 CPU cycle
    // OAM DMA is 513 CPU cycles + 1 if starting on CPU get cycle
    // Cycle reference:
    // https://www.nesdev.org/wiki/Cycle_reference_chart
    // Scanline timing:
    // https://www.nesdev.org/wiki/NTSC_video

    // BG and Sprite pixel
    uint8_t bg_px = 0;
    uint8_t spr_px = 0;
    uint8_t out = 0;
    // OA priority
    bool oa_prio = 0;

    // Background mux
    if (ppumask.bg_show) {
    }

    // Sprite 0..7
    if (ppumask.spr_show) {
    }

    // Priority mux
    if (!bg_px && !spr_px)
        out = 0x0;  // EXT in, which we won't have
    else if (!bg_px)
        out = spr_px;
    else if (bg_px && !spr_px)
        out = bg_px;
    else
        out = oa_prio ? spr_px : bg_px;

    // Increment scanline/pixel counter
    if (scan_x == (ntsc_x - 1)) scan_y = (scan_y + 1) % ntsc_y;
    scan_x = (scan_x + 1) % ntsc_x;

    if (false)
        if (draw_handler) draw_handler(pal.get_rgb(out));
}

void PPU::chr_write(uint16_t addr, uint8_t value) {
    throw std::runtime_error("PPU CHR write unimplemented");
}

uint8_t PPU::chr_read(uint16_t addr) {
    throw std::runtime_error("PPU CHR read unimplemented");
}

void PPU::cpu_write(uint16_t addr, uint8_t value) {
    switch (addr) {
        case 0x2000:
            ppuctrl.value = value;
            break;
        case 0x2001:
            ppumask.value = value;
            break;
        case 0x2002:
            throw std::runtime_error("Invalid write to PPUSTATUS.");
        case 0x2003:
            write_oamaddr(value);
            break;
        case 0x2004:
            write_oamdata(value);
            break;
        case 0x2005:
            write_ppuscroll(value);
            break;
        case 0x2006:
            write_ppuaddr(value);
            break;
        case 0x2007:
            write_ppudata(value);
            break;
        default:
            throw std::runtime_error("Invalid/unimplemented PPU write.");
            // TODO: Move OAMDMA $4014 here?
    }
}

uint8_t PPU::cpu_read(uint16_t addr) {
    switch (addr) {
        case 0x2002:
            return ppustatus.value;
        case 0x2004:
            // TODO: Reads while rendering should expose internal OAM accesses
            return oam[oamaddr];
        case 0x2007:
            uint8_t ppuval;
            if (ppuaddr16 >= 0x3F00) {
                ppuval = vram[0x3F00 + ((ppuaddr16 - 0x3F00) % 0x20)];
            } else {
                ppuval = vram[ppuaddr16];
            }
            if (ppuctrl.vram_addr_incr) ppuaddr16++;
            return ppuval;
        default:
            throw std::runtime_error("Invalid/unimplemented PPU write.");
            // TODO: Move OAMDMA $4014 here?
    }
    throw std::runtime_error("PPU read unimplemented.");
}

void PPU::write_ppuscroll(uint8_t value) {
    if (ppu_x)
        ppuscrollx = value;
    else
        ppuscrolly = value;
    ppu_x = !ppu_x;
}

void PPU::write_oamaddr(uint8_t value) { oamaddr = value; }

void PPU::write_oamdata(uint8_t value) { oam[oamaddr++] = value; }

void PPU::write_ppuaddr(uint8_t value) {
    if (ppu_h)
        ppuaddr16 = ((uint16_t)value << 8) | (ppuaddr16 & 0xFF);
    else
        ppuaddr16 = (ppuaddr16 & 0xFF00) | value;
    ppu_h = !ppu_h;
}

void PPU::write_ppudata(uint8_t value) {
    switch (ppuaddr16) {
        case 0x0 ... 0x3EFF:
            vram[ppuaddr16] = value;
            break;
        case 0x3F00 ... 0x3FFF:
            vram[0x3F00 + ((ppuaddr16 - 0x3F00) % 0x20)] = value;
            break;
        default:
            throw std::runtime_error("Invalid VRAM address");
    }
    if (ppuctrl.vram_addr_incr) ppuaddr16++;
}
