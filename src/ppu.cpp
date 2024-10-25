#include <ppu.h>
#include <render.h>

#include <cstring>

using namespace NES;

void inc_hori(PPUVramAddr &r) {
    r.sc_x++;
    if (!r.sc_x)
        r.nt_h = !r.nt_h;  // Not sure if the nametable switch is correct here
}

void inc_vert(PPUVramAddr &r) {
    r.sc_fine_y++;
    if (!r.sc_fine_y) r.sc_y++;
    if (!r.sc_y) r.nt_v = !r.nt_v;
}

void set_hori(PPUVramAddr &to, const PPUVramAddr &from) {
    to.sc_x = from.sc_x;
    to.nt_h = from.nt_h;
}

void set_vert(PPUVramAddr &to, const PPUVramAddr &from) {
    to.sc_y = from.sc_y;
    to.nt_v = from.nt_v;
    to.sc_fine_y = from.sc_fine_y;
}

PPU::PPU(GFX::Renderer &_renderer, NES::Palette _pal)
    : mapper(nullptr), renderer(_renderer), pal(std::move(_pal)) {
    power();
}

void PPU::power() {
    v.addr = 0;
    t.addr = 0;
    x.fine = 0;
    w = false;
    bus.addr = 0x0;
    cpu_bus = 0x0;
    nt = 0x0;
    at = 0x0;
    bg_l_shift = 0x0;
    bg_h_shift = 0x0;
    ppuctrl.value = 0x0;
    ppumask.value = 0x0;
    ppustatus.value = 0x0;
    oamaddr = 0x0;
    oamdata = 0x0;
    ppudata_buf = 0x0;
    scan_x = 0;
    scan_y = 0;
    scan_x_end = ntsc_x;
    scan_y_end = ntsc_y;
    scan_short = 1;
    std::fill(vram.begin(), vram.end(), 0x0);
    std::fill(oam.begin(), oam.end(), 0x3F);
    std::fill(oam_sec.begin(), oam_sec.end(), 0x3F);
    std::fill(pram.begin(), pram.end(), 0xFF);
    std::fill(fb.begin(), fb.end(), 0x001100FF);
    std::fill(fb_sec.begin(), fb_sec.end(), 0x001100FF);
}

void PPU::oam_sec_clear() { oam_sec[scan_x - 1] = 0xFF; }

void PPU::sprite_eval() {
    if (!ppumask.bg_show && !ppumask.spr_show) {
        return;
    }

    if (scan_x & 0x1) {
        oamdata = oam[oamaddr];
        return;
    }

    const OA *obj = nullptr;
    size_t stride = sizeof(OA);
    uint8_t tile_y = ppuctrl.spr_size ? 16 : 8;
    uint8_t spr_num = 0;
    for (size_t i = 0; i < (oam_sz / stride); i++) {
        obj = std::bit_cast<OA *>(oam.data() + i * stride);
        if (obj->y <= scan_y && (obj->y <= scan_y + tile_y)) {
            uint8_t *oam_target = (oam_sec.data() + spr_num * stride);
            std::memcpy(oam_target, obj, stride);
            spr_num++;
            if (spr_num >= 8) break;
        }
    }
}

void PPU::draw() {
    uint8_t out = 0;
    // uint8_t bg = 0;
    // uint8_t spr = 0;

    // Background
    if (ppumask.bg_show) {
        uint8_t bg = (((bg_l_shift << v.sc_x) & 0x8000) >> 15) |
                     (((bg_h_shift << v.sc_x) & 0x8000) >> 14);
        out = bg;
    }

    // Sprites
    if (ppumask.spr_show) {
        // TODO General
        //
        // TODO: Sprite 0 hit condition
        // Sprite 0 is in range
        // AND the first sprite output unit is outputting a non-zero
        // pixel AND the background drawing unit is outputting a
        // non-zero pixel
    }

    // Write to framebuffer
    int fb_i = scan_y * ntsc_fb_x + scan_x;
    if (fb_prim) {
        fb[fb_i] = pal.get_rgba(out);
    } else {
        fb_sec[fb_i] = pal.get_rgba(out);
    }

    // Shift tile
    bg_l_shift <<= 1;
    bg_h_shift <<= 1;
}

void PPU::execute(uint8_t cycles) {
    while (cycles) {
        switch (scan_y) {
        case 241:
            if (scan_x == 1) {
                ppustatus.vblank = true;
                if (on_nmi_vblank)
                    on_nmi_vblank();
                break;
            }
        case 0 ... 239:
        case 261:
            if (scan_x >= 1 && scan_x <= 64) oam_sec_clear();
            if (scan_x >= 65 && scan_x <= 256) sprite_eval();
            if (scan_x >= 257 && scan_x <= 320) oamaddr = 0x0;
            draw();
            if (scan_y == 261 && scan_x == 1) {
                ppustatus.vblank = false;
                ppustatus.spr0_hit = false;
            }
            if (scan_y == 261 && scan_x >= 280 && scan_x <= 304) {
                set_vert(v, t);
            }
            switch (scan_x) {
            case 0:
                if (scan_y == 0) {
                    // NT fetch happens here on odd frame
                } else if (scan_y != 261) {
                    // BG l addr
                }
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
                // clang-format off
                // NT
            case 9:     case 17:    case 25:    case 33:    case 41:    
            case 49:    case 57:    case 65:    case 73:    case 81:
            case 89:    case 97:    case 105:   case 113:   case 121:
            case 129:   case 137:   case 145:   case 153:   case 161:
            case 169:   case 177:   case 185:   case 193:   case 201:
            case 209:   case 217:   case 225:   case 233:   case 241:
            case 249:   case 257:   case 265:   case 273:   case 281:
            case 289:   case 297:   case 305:   case 313:   case 321:
            case 329:   case 337:   case 339:
                // clang-format on
                if (scan_x == 257) {
                    set_hori(v, t);
                }
                bus.addr = 0x2000 | (v.addr & 0x0FFF);
                break;
                // clang-format off
            case 2:     case 10:    case 18:    case 26:    case 34:
            case 42:    case 50:    case 58:    case 66:    case 74:
            case 82:    case 90:    case 98:    case 106:   case 114:
            case 122:   case 130:   case 138:   case 146:   case 154:
            case 162:   case 170:   case 178:   case 186:   case 194:
            case 202:   case 210:   case 218:   case 226:   case 234:
            case 242:   case 250:   case 258:   case 266:   case 274:
            case 282:   case 290:   case 298:   case 306:   case 314:
            case 322:   case 330:   case 338:   case 340:
                // clang-format on
                nt = read(bus.addr);
                break;
                // clang-format off
                // AT
            case 3:     case 11:    case 19:    case 27:    case 35:
            case 43:    case 51:    case 59:    case 67:    case 75:
            case 83:    case 91:    case 99:    case 107:   case 115:
            case 123:   case 131:   case 139:   case 147:   case 155:
            case 163:   case 171:   case 179:   case 187:   case 195:
            case 203:   case 211:   case 219:   case 227:   case 235:
            case 243:   case 251:   case 323:   case 331:
                // clang-format on
                bus.addr = 0x23C0 | (v.addr & 0x0C00) | ((v.addr >> 4) & 0x38) |
                           ((v.addr >> 2) & 0x07);
                break;
                // clang-format off
            case 4:     case 12:    case 20:    case 28:    case 36:
            case 44:    case 52:    case 60:    case 68:    case 76:
            case 84:    case 92:    case 100:   case 108:   case 116:
            case 124:   case 132:   case 140:   case 148:   case 156:
            case 164:   case 172:   case 180:   case 188:   case 196:
            case 204:   case 212:   case 220:   case 228:   case 236:
            case 244:   case 252:   case 324:   case 332:
                // clang-format on
                at = read(bus.addr);
                break;
                // clang-format off
                // BG L
            case 5:     case 13:    case 21:    case 29:    case 37:
            case 45:    case 53:    case 61:    case 69:    case 77:
            case 85:    case 93:    case 101:   case 109:   case 117:
            case 125:   case 133:   case 141:   case 149:   case 157:
            case 165:   case 173:   case 181:   case 189:   case 197:
            case 205:   case 213:   case 221:   case 229:   case 237:
            case 245:   case 253:   case 325:   case 333:
                // clang-format on
                bus.addr = (ppuctrl.spr_pt_addr ? 0x1000 : 0x0000) + nt * 16 +
                           v.sc_fine_y;
                break;
                // clang-format off
            case 6:     case 14:    case 22:    case 30:    case 38:
            case 46:    case 54:    case 62:    case 70:    case 78:
            case 86:    case 94:    case 102:   case 110:   case 118:
            case 126:   case 134:   case 142:   case 150:   case 158:
            case 166:   case 174:   case 182:   case 190:   case 198:
            case 206:   case 214:   case 222:   case 230:   case 238:
            case 246:   case 254:   case 326:   case 334:
                // clang-format on
                bg_l_shift |= read(bus.addr);
                break;
                // clang-format off
                // BG H
            case 7:     case 15:    case 23:    case 31:    case 39:
            case 47:    case 55:    case 63:    case 71:    case 79:
            case 87:    case 95:    case 103:   case 111:   case 119:
            case 127:   case 135:   case 143:   case 151:   case 159:
            case 167:   case 175:   case 183:   case 191:   case 199:
            case 207:   case 215:   case 223:   case 231:   case 239:
            case 247:   case 255:   case 327:   case 335:
                // clang-format on
                bus.addr = (ppuctrl.spr_pt_addr ? 0x1000 : 0x0000) + nt * 16 +
                           v.sc_fine_y + 8;
                break;
                // clang-format off
            case 8:     case 16:    case 24:    case 32:    case 40:
            case 48:    case 56:    case 64:    case 72:    case 80:
            case 88:    case 96:    case 104:   case 112:   case 120:
            case 128:   case 136:   case 144:   case 152:   case 160:
            case 168:   case 176:   case 184:   case 192:   case 200:
            case 208:   case 216:   case 224:   case 232:   case 240:
            case 248:   case 256:   case 328:   case 336:
                // clang-format on
                bg_h_shift |= read(bus.addr);
                if (scan_y == 256) {
                    inc_vert(v);
                }
                bg_l_shift <<= 8;
                bg_h_shift <<= 8;
                inc_hori(v);
                break;
            }
            break;
        }

        if (scan_y == 239 && scan_x == 320) {
            if (fb_prim)
                renderer.draw(fb.data());
            else
                renderer.draw(fb_sec.data());
            fb_prim = !fb_prim;
        }

        if (scan_x == ntsc_x - 1 && scan_y == ntsc_y - 1 && scan_short) {
            scan_short = !scan_short;
        }

        // Increment scanline/pixel counter
        if (scan_x == (ntsc_x - 1)) scan_y = (scan_y + 1) % ntsc_y;
        scan_x = (scan_x + 1) % ntsc_x;

        cycles--;
    }
}

void PPU::cpu_write(uint16_t addr, uint8_t value) {
    cpu_bus = value;
    switch (addr) {
    case 0x2000:  // PPUCTRL
        ppuctrl.value = value;
        break;
    case 0x2001:  // PPUMASK
        ppumask.value = value;
        break;
    case 0x2002:  // PPUSTATUS read-only
        break;
    case 0x2003:  // OAMADDR
        oamaddr = value;
        break;
    case 0x2004:  // OAMDATA
        oam[oamaddr++] = value;
        break;
    case 0x2005:  // PPUSCROLL
        if (!w) {
            t.sc_x = ((value & 0xF8) >> 3);
            x.fine = (value & 0x7);
        } else {
            t.sc_y = ((value & 0xF8) >> 3);
            t.sc_fine_y = (value & 0x7);
        }
        w = !w;
        break;
    case 0x2006:  // PPUADDR
        if (!w)
            v.h = value & 0x3F;
        else
            v.l = value;
        w = !w;
        break;
    case 0x2007:  // PPUDATA
        write(v.addr, value);
        if (ppuctrl.v_incr) {
            v.addr += 0x20;
        } else {
            v.addr++;
        }
        break;
    default: throw std::runtime_error("Invalid/unimplemented PPU write.");
    }
}

uint8_t PPU::cpu_read(uint16_t addr) {
    switch (addr) {
    case 0x2000:  // Write-only
        return cpu_bus;
    case 0x2001:  // Write-only
        return cpu_bus;
    case 0x2002:
        w = (bool)0;
        // TODO: Reads from here should reset vblank flag bit 7
        cpu_bus = (ppustatus.value & 0xE0) | (cpu_bus & 0x1F);
        return ppustatus.value;
    case 0x2003:  // Write-only
        return cpu_bus;
    case 0x2004:
        // TODO: Reads while rendering should expose internal OAM accesses
        cpu_bus = oam[oamaddr];
        return oam[oamaddr];
    case 0x2005:
    case 0x2006: return cpu_bus;
    case 0x2007:
        uint8_t ppudata_out;
        if (v.addr > 0x3EFF) {
            ppudata_out = read(v.addr);
        } else {
            ppudata_out = ppudata_buf;
            ppudata_buf = read(v.addr);
        }
        if (ppuctrl.v_incr) {
            v.addr += 0x20;
        } else {
            v.addr++;
        }
        cpu_bus = ppudata_out;
        return ppudata_out;
    default:
        throw std::runtime_error("Invalid/unimplemented PPU write.");
        // TODO: Move OAMDMA $4014 here?
    }
    throw std::runtime_error("PPU read unimplemented.");
}

// https://fceux.com/web/help/PPU.html
// https://www.nesdev.org/wiki/PPU_programmer_reference
//
// PPU memory map
//
// $0000-$0FFF - CHR ROM/RAM Pattern table 0 (16 bit 2 plane patterns) (cart)
// $1000-$1FFF - CHR ROM/RAM Pattern table 1 (cart)
// $2000-$23FF - Nametable 0 (PPU VRAM)
// $2400-$27FF - Nametable 1 (PPU VRAM)
// $2800-$2BFF - Nametable 2 (mirrored or additional VRAM on cartridge)
// $2C00-$2FFF - Nametable 3 (mirrored or additional VRAM on cartridge)
// $3000-$3EFF - Mirrors of $2000-$2EFF
// $3F00-$3F1F - Palette RAM indices (PPU VRAM)
// $3F20-$3FFF - Palette RAM mirrors

void PPU::write(uint16_t addr, uint8_t value) {
    using namespace iNESv1::Mapper;
    NTMirror mirror = mapper->mirroring();
    switch (addr) {
    case 0x0000 ... 0x1FFF: mapper->write_ppu(addr, value); break;
    case 0x2000 ... 0x23FF: vram[addr - 0x2000] = value; break;
    case 0x2400 ... 0x27FF:
        switch (mirror) {
        case map_hori:
        case map_quad: vram[addr - 0x2000] = value; break;
        case map_vert:
        case map_single: vram[addr - 0x2400] = value; break;
        }
        break;
    case 0x2800 ... 0x2BFF:
        switch (mirror) {
        case map_hori:
        case map_single: vram[addr - 0x2800] = value; break;
        case map_vert: vram[addr - 0x2400] = value; break;
        case map_quad: mapper->write_ppu(addr, value); break;
        }
        break;
    case 0x2C00 ... 0x2FFF:
        switch (mirror) {
        case map_hori:
        case map_vert: vram[addr - 0x2800] = value; break;
        case map_single: vram[addr - 0x2C00] = value; break;
        case map_quad: mapper->write_ppu(addr, value); break;
        }
        break;
    case 0x3F00 ... 0x3FFF: pram[((addr - 0x3F00) % 0x20)] = value; break;
    default: throw std::runtime_error("Invalid/unimplemented PPU write");
    }
}

uint8_t PPU::read(uint16_t addr) {
    using namespace iNESv1::Mapper;
    NTMirror mirror = mapper->mirroring();
    switch (addr) {
    case 0x0000 ... 0x1FFF: return mapper->read_ppu(addr); break;
    case 0x2000 ... 0x23FF: return vram[addr - 0x2000]; break;
    case 0x2400 ... 0x27FF:
        switch (mirror) {
        case map_hori:
        case map_quad: return vram[addr - 0x2000]; break;
        case map_vert:
        case map_single: return vram[addr - 0x2400]; break;
        }
        break;
    case 0x2800 ... 0x2BFF:
        switch (mirror) {
        case map_hori:
        case map_single: return vram[addr - 0x2800]; break;
        case map_vert: return vram[addr - 0x2400]; break;
        case map_quad: return mapper->read_ppu(addr); break;
        }
        break;
    case 0x2C00 ... 0x2FFF:
        switch (mirror) {
        case map_hori:
        case map_vert: return vram[addr - 0x2800]; break;
        case map_single: return vram[addr - 0x2C00]; break;
        case map_quad: return mapper->read_ppu(addr); break;
        }
        break;
    case 0x3F00 ... 0x3FFF: return pram[((addr - 0x3F00) % 0x20)]; break;
    default: break;
    }
    throw std::runtime_error("Invalid or unimplemented PPU read");
}
