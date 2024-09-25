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

void inc_hori(PPUVramAddr &r) {
    r.sc_x++;
    if (!r.sc_x) r.nt_h = !r.nt_h;
}

void inc_vert(PPUVramAddr &r) {
    r.sc_fine_y++;
    if (!r.sc_fine_y) r.sc_y++;
    if (!r.sc_y) r.nt_v = !r.nt_v;
}

void set_hori(PPUVramAddr &to, PPUVramAddr &from) {
    to.sc_x = from.sc_x;
    to.nt_h = from.nt_h;
}

void set_vert(PPUVramAddr &to, PPUVramAddr &from) {
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
    bus.addr = 0x0;
    nt = 0x0;
    at = 0x0;
    spr_l = 0x0;
    spr_h = 0x0;
    ppuctrl.value = 0x0;
    ppumask.value = 0x0;
    ppustatus.value = 0x0;
    oamaddr = 0x0;
    ppudata_buf = 0x0;
    scan_y = 0;
    scan_x = 0;
}

void PPU::execute(uint8_t cycles) {
    while (cycles) {
        switch (scan_y) {
//             case 0 ... 19:
//                 // pull down VINT
//                 // no accesses to PPU external memory
//                 // /VBL issues zero level and is tied to 2a03 /NMI line
//                 if (scan_x == 0) {
//                     // TODO: Theoretically vbl_nmi NAND vblank. Is this
//                     // actually correct?
//                     if (!(ppuctrl.vbl_nmi && ppustatus.vblank))
//                         if (nmi_vblank) nmi_vblank();
//                 }
//                 cycles--;
//                 break;
//             case 20 ... 260:
//                 render();
//                 cycles--;
//                 break;
            case 0 ... 239:
            case 261:
                switch (scan_x) {
                    case 0:
                        switch (scan_y) {
                            case 0:
                                // TODO: Skipped on BG+odd
                                break;
                            case 1 ... 239:
                                // TODO: BG l address only
                                break;
                        }
                    case 1:
                        if (scan_y == 261) {
                            // TODO: Clear vblank flag and spr0 overflow
                            ppustatus.vblank = false;
                            ppustatus.spr0_hit = false;
                        }
                        if (scan_y == 241) {
                            // TODO: Is this the right flag or the NMI one?
                            ppustatus.vblank = true;
                        }
                    // NT fetch
                    case 9:
                    case 17:
                    case 25:
                    case 33:
                    case 41:
                    case 49:
                    case 57:
                    case 65:
                    case 73:
                    case 81:
                    case 89:
                    case 97:
                    case 105:
                    case 113:
                    case 121:
                    case 129:
                    case 137:
                    case 145:
                    case 153:
                    case 161:
                    case 169:
                    case 177:
                    case 185:
                    case 193:
                    case 201:
                    case 209:
                    case 217:
                    case 225:
                    case 233:
                    case 241:
                    case 249:
                    case 321: // Two tiles from next scanline
                    case 329: // Two tiles from next scanline
                    case 337: // TODO: Unused?
                    case 339: // TODO: Unused?
                        bus.addr = 0x2000 | (v.addr & 0x0FFF);
                        break;
                    // NT read
                    case 2:
                    case 10:
                    case 18:
                    case 26:
                    case 34:
                    case 42:
                    case 50:
                    case 58:
                    case 66:
                    case 74:
                    case 82:
                    case 90:
                    case 98:
                    case 106:
                    case 114:
                    case 122:
                    case 130:
                    case 138:
                    case 146:
                    case 154:
                    case 162:
                    case 170:
                    case 178:
                    case 186:
                    case 194:
                    case 202:
                    case 210:
                    case 218:
                    case 226:
                    case 234:
                    case 242:
                    case 250:
                    case 322: // Two tiles from next scanline
                    case 330: // Two tiles from next scanline
                    case 338: // TODO: Unused?
                    case 340: // TODO: Unused?
                        nt = vram[bus.addr];
                        break;
                    // AT fetch
                    case 3:
                    case 11:
                    case 19:
                    case 27:
                    case 35:
                    case 43:
                    case 51:
                    case 59:
                    case 67:
                    case 75:
                    case 83:
                    case 91:
                    case 99:
                    case 107:
                    case 115:
                    case 123:
                    case 131:
                    case 139:
                    case 147:
                    case 155:
                    case 163:
                    case 171:
                    case 179:
                    case 187:
                    case 195:
                    case 203:
                    case 211:
                    case 219:
                    case 227:
                    case 235:
                    case 243:
                    case 251:
                    case 323: // Two tiles from next scanline
                    case 331: // Two tiles from next scanline
                        bus.addr = 0x23C0 \
                                   | (v.addr & 0x0C00) \
                                   | ((v.addr >> 4) & 0x38) \
                                   | ((v.addr >> 2) & 0x07);
                        break;
                    // AT read
                    case 4:
                    case 12:
                    case 20:
                    case 28:
                    case 36:
                    case 44:
                    case 52:
                    case 60:
                    case 68:
                    case 76:
                    case 84:
                    case 92:
                    case 100:
                    case 108:
                    case 116:
                    case 124:
                    case 132:
                    case 140:
                    case 148:
                    case 156:
                    case 164:
                    case 172:
                    case 180:
                    case 188:
                    case 196:
                    case 204:
                    case 212:
                    case 220:
                    case 228:
                    case 236:
                    case 244:
                    case 252:
                    case 324: // Two tiles from next scanline
                    case 332: // Two tiles from next scanline
                        at = vram[v.addr];
                        break;
                    // BG l fetch
                    case 5:
                    case 13:
                    case 21:
                    case 29:
                    case 37:
                    case 45:
                    case 53:
                    case 61:
                    case 69:
                    case 77:
                    case 85:
                    case 93:
                    case 101:
                    case 109:
                    case 117:
                    case 125:
                    case 133:
                    case 141:
                    case 149:
                    case 157:
                    case 165:
                    case 173:
                    case 181:
                    case 189:
                    case 197:
                    case 205:
                    case 213:
                    case 221:
                    case 229:
                    case 237:
                    case 245:
                    case 253:
                    case 325: // Two tiles from next scanline
                    case 333: // Two tiles from next scanline
                        v.addr = (ppuctrl.spr_pt_addr ? 0x1000 : 0x0000) 
                                 + nt * 16 + v.sc_fine_y; 
                        break;
                    // BG l read
                    case 6:
                    case 14:
                    case 22:
                    case 30:
                    case 38:
                    case 46:
                    case 54:
                    case 62:
                    case 70:
                    case 78:
                    case 86:
                    case 94:
                    case 102:
                    case 110:
                    case 118:
                    case 126:
                    case 134:
                    case 142:
                    case 150:
                    case 158:
                    case 166:
                    case 174:
                    case 182:
                    case 190:
                    case 198:
                    case 206:
                    case 214:
                    case 222:
                    case 230:
                    case 238:
                    case 246:
                    case 254:
                    case 326: // Two tiles from next scanline
                    case 334: // Two tiles from next scanline
                        spr_l = vram[v.addr]; 
                        break;
                    // BG h fetch
                    case 7:
                    case 15:
                    case 23:
                    case 31:
                    case 39:
                    case 47:
                    case 55:
                    case 63:
                    case 71:
                    case 79:
                    case 87:
                    case 95:
                    case 103:
                    case 111:
                    case 119:
                    case 127:
                    case 135:
                    case 143:
                    case 151:
                    case 159:
                    case 167:
                    case 175:
                    case 183:
                    case 191:
                    case 199:
                    case 207:
                    case 215:
                    case 223:
                    case 231:
                    case 239:
                    case 247:
                    case 255:
                    case 327: // Two tiles from next scanline
                    case 335: // Two tiles from next scanline
                        v.addr = (ppuctrl.spr_pt_addr ? 0x1000 : 0x0000) 
                                 + nt * 16 + v.sc_fine_y + 8; 
                        break;
                    // BG h read
                    case 8:
                    case 16:
                    case 24:
                    case 32:
                    case 40:
                    case 48:
                    case 56:
                    case 64:
                    case 72:
                    case 80:
                    case 88:
                    case 96:
                    case 104:
                    case 112:
                    case 120:
                    case 128:
                    case 136:
                    case 144:
                    case 152:
                    case 160:
                    case 168:
                    case 176:
                    case 184:
                    case 192:
                    case 200:
                    case 208:
                    case 216:
                    case 224:
                    case 232:
                    case 240:
                    case 248:
                    case 256:
                    case 328: // Two tiles from next scanline
                    case 336: // Two tiles from next scanline 
                        spr_h = vram[v.addr];
                        if (scan_y == 256) {
                            inc_vert(v);
                        }
                        inc_hori(v);
                        break;
                }
                cycles--;
                break;
        }

        if (scan_y == 239 && scan_x == 320)
            if (frame_ready) 
                frame_ready(fb);

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
            if (v.addr > 0x3EFF) {
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
    if (!w)
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
