#include <ppu.h>

#include <cstring>

// https://fceux.com/web/help/PPU.html
// https://www.nesdev.org/wiki/PPU_programmer_reference
//
// PPU memory map
//
// Mapped by cartridge:
// $0000-$0FFF - CHR (Pattern) table 0 (16 bit 2 plane patterns)
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

void inc_hori(PPURegister &r) {
    r.sc_x++;
    if (!r.sc_x) r.nt_h = !r.nt_h;
}

void inc_vert(PPURegister &r) {
    r.sc_fine_y++;
    if (!r.sc_fine_y) r.sc_y++;
    if (!r.sc_y) r.nt_v = !r.nt_v;
}

void set_hori(PPURegister &to, PPURegister &from) {
    to.sc_x = from.sc_x;
    to.nt_h = from.nt_h;
}

void set_vert(PPURegister &to, PPURegister &from) {
    to.sc_y = from.sc_y;
    to.nt_v = from.nt_v;
    to.sc_fine_y = from.sc_fine_y;
}

PPU::PPU(NES::Palette _pal) : pal(std::move(_pal)) {}

void PPU::power() {
    v.addr = 0;
    t.addr = 0;
    x.fine = 0;
    w = false;
    ppuctrl.value = 0x0;
    ppumask.value = 0x0;
    ppustatus.value = 0x0;
    oamaddr = 0x0;
    ppudata_buf = 0x0;
    scan_y = 0;
    scan_x = 0;
}

void PPU::vram_fetch_nt() {}

void PPU::vram_fetch_at() {}

void PPU::vram_fetch_bg_l() {}

void PPU::vram_fetch_bg_h() {}

void PPU::vram_fetch_spr_l() {}

void PPU::vram_fetch_spr_h() {}

void PPU::execute(uint8_t cycles) {
    while (cycles) {
        switch (scan_y) {
            case 0 ... 19:
                // pull down VINT
                // no accesses to PPU external memory
                // /VBL issues zero level and is tied to 2a03 /NMI line
                if (scan_x == 0) {
                    // TODO: Theoretically vbl_nmi NAND vblank. Is this
                    // actually correct?
                    if (!(ppuctrl.vbl_nmi && ppustatus.vblank))
                        if (nmi_vblank) nmi_vblank();
                }
                cycles--;
                break;
            case 20 ... 260:
                render();
                cycles--;
                break;
            case 261:
                if (scan_x == (ntsc_x - 1)) {
                    ppustatus.vblank = true;
                }
                cycles--;
                break;
        }

        // TODO: Actually render some output
        if (false)
            if (frame_ready) frame_ready(fb);

        // Increment scanline/pixel counter
        if (scan_x == (ntsc_x - 1)) scan_y = (scan_y + 1) % ntsc_y;
        scan_x = (scan_x + 1) % ntsc_x;
    }
}

void PPU::render() {
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
    // TODO: This is object specific, set it after sprite evaluation
    bool oa_prio = 0;

    // Background pixel mux
    if (ppumask.bg_show) {
        // 1. Fetch a nametable entry from $2000-$2FFF.
        // 2. Fetch the corresponding attribute table entry from $23C0-$2FFF
        //  and increment the current VRAM address within the same row.
        // 3. Fetch the low-order byte of an 8x1 pixel sliver of pattern table
        //  from $0000-$0FF7 or $1000-$1FF7.
        // 4. Fetch the high-order byte of this sliver from an address 8 bytes
        //  higher.
        // 5. Turn the attribute data and the pattern table data into palette
        //  indices, and combine them with data from sprite data using priority
    }

    // Sprite unit output
    if (ppumask.spr_show) {
        // Sprite evaluation
        OA *obj = nullptr;
        size_t stride = sizeof(OA);
        uint8_t tile_y = ppuctrl.spr_size ? 16 : 8;
        uint8_t spr_num = 0;
        for (size_t i = 0; i < (oam_sz / stride); i++) {
            obj = (OA *)(oam.data() + i * stride);
            if (obj->y <= scan_y && (obj->y <= scan_y + tile_y)) {
                uint8_t *oam_target = (oam_sec.data() + spr_num * stride);
                // TODO: Sprite 0 hit condition
                // Sprite 0 is in range
                // AND the first sprite output unit is outputting a non-zero
                // pixel AND the background drawing unit is outputting a
                // non-zero pixel
                std::memcpy(oam_target, obj, stride);
                spr_num++;
                if (spr_num >= 8) break;
            }
        }

        // Sprite unit priority, lower addr overlaps higher addr
    }

    // Priority mux
    if (!bg_px && !spr_px)
        out = 0x0;  // Backdrop color $3F00 EXT in
    else if (!bg_px)
        out = spr_px;
    else if (bg_px && !spr_px)
        out = bg_px;
    else
        out = oa_prio ? spr_px : bg_px;

    if (out) {
    }
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
            w = (bool)0;
            // TODO: Reads from here should reset vblank flag bit 7
            return ppustatus.value;
        case 0x2004:
            // TODO: Reads while rendering should expose internal OAM accesses
            return oam[oamaddr];
        case 0x2007:
            uint8_t ppudata_out;
            if (v.addr >= 0x3EFF) {
                ppudata_out = vram[0x3F00 + ((v.addr - 0x3F00) % 0x20)];
            } else {
                ppudata_out = ppudata_buf;
                ppudata_buf = vram[v.addr];
            }
            if (ppuctrl.v_incr) {
                v.addr += 0x20;
            } else {
                v.addr++;
            }
            return ppudata_out;
        default:
            throw std::runtime_error("Invalid/unimplemented PPU write.");
            // TODO: Move OAMDMA $4014 here?
    }
    throw std::runtime_error("PPU read unimplemented.");
}

void PPU::write_ppuscroll(uint8_t value) {
    if (!w) {
        t.sc_x = ((value & 0xF8) >> 3);
        x.fine = (value & 0x7);
    } else {
        t.sc_y = ((value & 0xF8) >> 3);
        t.sc_fine_y = (value & 0x7);
    }
    w = !w;
}

void PPU::write_oamaddr(uint8_t value) { oamaddr = value; }

void PPU::write_oamdata(uint8_t value) { oam[oamaddr++] = value; }

void PPU::write_ppuaddr(uint8_t value) {
    if (!w)  // First write
        t.h = value & 0x3F;
    else
        t.l = value;
    w = !w;
}

void PPU::write_ppudata(uint8_t value) {
    // - Because the PPU cannot make a read from PPU memory immediately upon
    // request (via $2007), there is an internal buffer, which acts as a 1-stage
    // data pipeline. As a read is requested, the contents of the read buffer
    // are returned to the NES's CPU. After this, at the PPU's earliest
    // convience (according to PPU read cycle timings), the PPU will fetch the
    // requested data from the PPU memory, and throw it in the read buffer.
    // Writes to PPU mem via $2007 are pipelined as well, but I currently haven
    // unknown to me if the PPU uses this same buffer (this could be easily
    // tested by writing somthing to $2007, and seeing if the same value is
    // returned immediately after reading).
    switch (v.addr) {
        case 0x0 ... 0x3EFF:
            vram[v.addr] = value;
            break;
        case 0x3F00 ... 0x3FFF:
            vram[0x3F00 + ((v.addr - 0x3F00) % 0x20)] = value;
            break;
        default:
            throw std::runtime_error("Invalid VRAM address");
    }

    if (ppuctrl.v_incr) {
        v.addr += 0x20;
    } else {
        v.addr++;
    }
}
