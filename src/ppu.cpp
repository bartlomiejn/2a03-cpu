#include <gui.h>
#include <log.h>
#include <ppu.h>

#include <cstring>
#include <format>
#include <iomanip>

using namespace std;
using namespace NES;

void inc_hori(PPUVramAddr &r) {
    if (r.sc_x == 0x1F) {
        r.sc_x = 0;
        r.nt_h ^= 1;  // Switch H nametable
    } else {
        r.sc_x++;
    }
}

void inc_vert(PPUVramAddr &r) {
    // https://www.nesdev.org/wiki/PPU_scrolling#Wrapping_around
    if (r.sc_fine_y < 7) {
        r.sc_fine_y++;
    } else {
        r.sc_fine_y = 0;

        if (r.sc_y == 29) {
            r.sc_y = 0;
            r.nt_v ^= 1;  // Switch V nametable
        } else if (r.sc_y == 31) {
            r.sc_y = 0;
        } else {
            r.sc_y++;
        }
    }
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

PPU::PPU(GFX::GUI &_gui, NES::Palette _pal)
    : mapper(nullptr), gui(_gui), pal(std::move(_pal)) {
    power();
}

void PPU::power() {
    NES_LOG("PPU") << "Power on" << std::endl;
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
    oam_overflow = false;
    oam_sec_overflow = false;
    spr0_in_range = false;
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
    spr_out.fill({0, 0, 0, 0});
    std::fill(fb.begin(), fb.end(), 0x000000FF);
    std::fill(fb_sec.begin(), fb_sec.end(), 0x000000FF);
}

void PPU::oam_sec_clear() {
    uint8_t addr = (scan_x - 1) % oam_sec_sz;
    NES_LOG("PPU") << std::format("Clear secondary OAM, oam_sec@{:02X}=FF\n",
                                  addr);
    oam_sec[addr] = 0xFF;
}

void PPU::sprite_eval() {
    if (!ppumask.bg_show && !ppumask.spr_show) {
        return;
    }

    // Odd cycles: read from OAM into OAMDATA register
    if (scan_x & 0x1) {
        oamdata = oam[oamaddr];
        NES_LOG("PPU") << std::format(
            "Sprite eval, set OAMDATA to OAM@{:02X}={:02X}\n", oamaddr,
            oamdata);
        return;
    }

    uint8_t spr_height = ppuctrl.spr_size ? 16 : 8;
    uint8_t spr_count = 0;
    spr0_in_range = false;

    for (uint8_t n = 0; n < 64; n++) {
        uint8_t sprite_y = oam[n * 4];

        if (scan_y >= sprite_y && scan_y < sprite_y + spr_height) {
            if (spr_count < 8) {
                if (n == 0) spr0_in_range = true;
                NES_LOG("PPU") << std::format(
                    "Sprite eval: Sprite #{:d} is on next scanline\n", n);
                std::memcpy(
                    oam_sec.data() + spr_count * 4,
                    oam.data() + n * 4,
                    4
                );
                spr_count++;
            } else {
                NES_LOG("PPU") << "Sprite eval: Overflow\n";
                // TODO: Hardware bug - after finding 8 sprites, the PPU
                // increments the byte offset within OAM entries incorrectly,
                // causing false negatives. Few games rely on this.
                ppustatus.spr_overflow = true;
                break;
            }
        }
    }
}

void PPU::sprite_fetch() {
    if (!ppumask.bg_show && !ppumask.spr_show) return;

    uint8_t spr_height = ppuctrl.spr_size ? 16 : 8;

    for (int i = 0; i < 8; i++) {
        uint8_t sprite_y  = oam_sec[i * 4 + 0];
        uint8_t tile_idx  = oam_sec[i * 4 + 1];
        uint8_t attr      = oam_sec[i * 4 + 2];
        uint8_t sprite_x  = oam_sec[i * 4 + 3];

        spr_out[i].attr = attr;
        spr_out[i].x = sprite_x;

        // Empty slot (secondary OAM cleared to 0xFF)
        if (sprite_y == 0xFF) {
            spr_out[i].pat_l = 0;
            spr_out[i].pat_h = 0;
            continue;
        }

        uint8_t row = scan_y - sprite_y;

        // Vertical flip
        if (attr & 0x80) {
            row = (spr_height - 1) - row;
        }

        uint16_t tile_addr;
        if (!ppuctrl.spr_size) {
            // 8x8: pattern table from PPUCTRL bit 3
            tile_addr = (ppuctrl.spr_pt_addr ? 0x1000 : 0x0000)
                      | (tile_idx << 4)
                      | row;
        } else {
            // 8x16: pattern table from tile index bit 0
            uint16_t pt = (tile_idx & 1) ? 0x1000 : 0x0000;
            uint8_t tile_num = tile_idx & 0xFE;
            if (row >= 8) {
                tile_num++;
                row -= 8;
            }
            tile_addr = pt | (tile_num << 4) | row;
        }

        uint8_t pat_l = read(tile_addr);
        uint8_t pat_h = read(tile_addr + 8);

        // Horizontal flip: reverse bit order
        if (attr & 0x40) {
            pat_l = (pat_l & 0xF0) >> 4 | (pat_l & 0x0F) << 4;
            pat_l = (pat_l & 0xCC) >> 2 | (pat_l & 0x33) << 2;
            pat_l = (pat_l & 0xAA) >> 1 | (pat_l & 0x55) << 1;
            pat_h = (pat_h & 0xF0) >> 4 | (pat_h & 0x0F) << 4;
            pat_h = (pat_h & 0xCC) >> 2 | (pat_h & 0x33) << 2;
            pat_h = (pat_h & 0xAA) >> 1 | (pat_h & 0x55) << 1;
        }

        spr_out[i].pat_l = pat_l;
        spr_out[i].pat_h = pat_h;
    }
}

void PPU::draw() {
    uint32_t out[8] = {0};
    uint8_t bg_color[8] = {0};
    uint8_t at_x, at_y = 0;
    uint8_t at_pal = 0;

    // Background
    at_x = (((scan_x - 1) % 16) / 8) * 2;
    at_y = ((scan_y % 16) / 8) * 4;
    at_pal = (at >> (at_y + at_x)) & 0x3;

    NES_LOG("PPU") << std::format("Draw BG: at_x: {:d} at_y {:d} at_pal {:d}\n",
                                  at_x, at_y, at_pal);

    if (ppumask.bg_show) {
        uint16_t pix_mask = 0x8000;

        NES_LOG("PPU") << std::format(
            "BGL: 0x{:04X} BGH: {:04X} x.fine: 0x {:02X}\n", bg_l_shift,
            bg_h_shift, (uint16_t)x.fine);

        for (int i = 0; i < 8; i++) {
            uint8_t bg = ((bg_l_shift << x.fine) & pix_mask) >> (15 - i) |
                         ((bg_h_shift << x.fine) & pix_mask) >> (14 - i);
            NES_LOG("PPU") << std::format("bg: {:d}, pram@{:02X}={:02X}\n", bg,
                                          (bg | at_pal << 2),
                                          pram[bg | (at_pal << 2)]);
            bg_color[i] = bg;
            out[i] = pal.get_rgba(pram[bg | (at_pal << 2)]);
            pix_mask >>= 1;
        }
    }

    // Sprites
    if (ppumask.spr_show) {
        uint16_t px_base = scan_x - 1;

        for (int i = 0; i < 8; i++) {
            uint16_t px = px_base + i;

            for (int s = 0; s < 8; s++) {
                uint8_t spr_x = spr_out[s].x;

                if (px < spr_x || px >= spr_x + 8) continue;

                uint8_t bit = 7 - (px - spr_x);
                uint8_t color = ((spr_out[s].pat_h >> bit) & 1) << 1
                              | ((spr_out[s].pat_l >> bit) & 1);

                if (color == 0) continue;

                // Sprite 0 hit: spr0 in range, first secondary OAM entry,
                // both BG and sprite pixels non-transparent, x != 255,
                // both rendering flags enabled
                if (s == 0 && spr0_in_range && bg_color[i] != 0
                    && px != 255 && ppumask.bg_show) {
                    ppustatus.spr0_hit = true;
                }

                bool behind_bg = spr_out[s].attr & 0x20;
                uint8_t spr_pal = spr_out[s].attr & 0x03;

                if (!behind_bg || bg_color[i] == 0) {
                    out[i] = pal.get_rgba(pram[0x10 | (spr_pal << 2) | color]);
                }

                break;  // First non-transparent sprite wins
            }
        }
    }

    int y_offset = scan_y * ntsc_fb_x;
    int x_offset = scan_x - 1;
    int offset = y_offset + x_offset;
    uint32_t *fb_ptr = fb_prim ? fb.data() : fb_sec.data();
    std::memcpy(fb_ptr + offset, out, sizeof(uint32_t) * 8);
}

void PPU::execute(uint16_t cycles) {
    NES_LOG("PPU") << "Run for " << dec << cycles << " cycles" << endl;
    while (cycles) {
        NES_LOG("PPU") << std::format(
            "X: {:d} Y: {:d} v: {:04X} t: {:04X} w: {:d}\n", scan_x, scan_y,
            (uint16_t)v.addr, (uint16_t)t.addr, w);
        if (scan_y == 241 && scan_x == 1) {
            NES_LOG("PPU") << "set vblank" << std::endl;
            ppustatus.vblank = true;
            if (ppuctrl.vbl_nmi && on_nmi_vblank) on_nmi_vblank();
        }
        if (scan_y <= 240 && scan_x == 0 &&
            (ppumask.bg_show || ppumask.spr_show)) {
            bus.addr = (ppuctrl.bg_pt_addr ? 0x1000 : 0x0000) | (nt << 4) |
                       v.sc_fine_y;
        }

        // Draw 8 pixels
        if (scan_y <= 239) {
            if (scan_x >= 1 && scan_x <= 249) {
                if (!((scan_x - 1) % 8)) draw();
            }
        }

        bg_l_shift <<= 1;
        bg_h_shift <<= 1;

        if (scan_y <= 239 || scan_y == 261) {
            // Clear flags
            if (scan_y == 261 && scan_x == 1) {
                NES_LOG("PPU") << "Clear flags" << endl;
                ppustatus.vblank = false;
                ppustatus.spr_overflow = false;
                ppustatus.spr0_hit = false;
            }

            // Sprite logic
            if (scan_y <= 239) {
                if (scan_x >= 1 && scan_x <= 64) oam_sec_clear();
                if (scan_x == 65) {
                    oam_overflow = false;
                    oam_sec_overflow = false;
                    oam_sec_addr = 0x0;
                }
                // Sprite evaluation happens on 65-256, same with fetch. Just
                // batch it for now, might not be needed to be more correct
                if (scan_x == 65) sprite_eval();
                if (scan_x == 257) sprite_fetch();
            }

            // Clear oamaddr
            if (scan_x >= 257 && scan_x <= 320) oamaddr = 0x0;

            // BG logic
            switch (scan_x) {
                // clang-format off
                // NT
            case 1:     case 9:     case 17:    case 25:    case 33:
            case 41:    case 49:    case 57:    case 65:    case 73:
            case 81:    case 89:    case 97:    case 105:   case 113:
            case 121:   case 129:   case 137:   case 145:   case 153:
            case 161:   case 169:   case 177:   case 185:   case 193:
            case 201:   case 209:   case 217:   case 225:   case 233:
            case 241:   case 249:   case 257:   case 259:   case 265:
            case 273:   case 281:   case 289:   case 297:   case 305:
            case 313:   case 321:   case 329:   case 337:   case 339:
                // clang-format on
                if (!ppumask.bg_show && !ppumask.spr_show) break;
                bus.addr = 0x2000 | (v.addr & 0x0FFF);
                NES_LOG("PPU") << "NT addr: 0x" << hex << setfill('0') << setw(4)
                     << bus.addr << endl;
                break;
                // clang-format off
            case 2:     case 10:    case 18:    case 26:    case 34:
            case 42:    case 50:    case 58:    case 66:    case 74:
            case 82:    case 90:    case 98:    case 106:   case 114:
            case 122:   case 130:   case 138:   case 146:   case 154:
            case 162:   case 170:   case 178:   case 186:   case 194:
            case 202:   case 210:   case 218:   case 226:   case 234:
            case 242:   case 250:   case 258:   case 260:   case 266:
            case 274:   case 282:   case 290:   case 298:   case 306:
            case 314:   case 322:   case 330:   case 338:   case 340:
                // clang-format on
                if (!ppumask.bg_show && !ppumask.spr_show) break;
                nt = read(bus.addr);
                NES_LOG("PPU") << "Read NT@0x" << hex << setfill('0') << setw(4)
                     << bus.addr << ": 0x" << setw(2) << (uint16_t)nt << endl;
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
                if (!ppumask.bg_show && !ppumask.spr_show) break;
                bus.addr = 0x23C0 | (v.addr & 0x0C00) | ((v.addr >> 4) & 0x38) |
                           ((v.addr >> 2) & 0x07);
                NES_LOG("PPU") << "AT addr: 0x" << hex << setfill('0') << setw(4)
                     << bus.addr << endl;
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
                if (!ppumask.bg_show && !ppumask.spr_show) break;
                at = read(bus.addr);
                NES_LOG("PPU") << "Read AT@0x" << hex << setfill('0') << setw(4)
                     << bus.addr << ": 0x" << setw(2) << (uint16_t)at << endl;
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
                if (!ppumask.bg_show && !ppumask.spr_show) break;
                bus.addr = (ppuctrl.bg_pt_addr ? 0x1000 : 0x0000) | (nt << 4) |
                           (v.sc_fine_y);
                NES_LOG("PPU") << "BGL addr: 0x" << hex << setfill('0') << setw(4)
                     << bus.addr << endl;
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
                if (!ppumask.bg_show && !ppumask.spr_show) break;
                bg_l_shift |= read(bus.addr);

                NES_LOG("PPU") << "Read BGL@0x" << hex << setfill('0') << setw(4)
                     << bus.addr << ", BGL SR = 0x" << setw(4)
                     << (uint16_t)bg_l_shift << endl;
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
                if (!ppumask.bg_show && !ppumask.spr_show) break;
                bus.addr = ((ppuctrl.bg_pt_addr ? 0x1000 : 0x0000) | (nt << 4) |
                            (v.sc_fine_y)) +
                           8;
                NES_LOG("PPU") << "BGH addr: 0x" << hex << setfill('0') << setw(4)
                     << bus.addr << endl;
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
                if (!ppumask.bg_show && !ppumask.spr_show) break;

                bg_h_shift |= (uint16_t)read(bus.addr) << 2;

                NES_LOG("PPU") << "Read BGH@0x" << hex << setfill('0') << setw(4)
                     << bus.addr << ", BGH SR = 0x" << setw(4)
                     << (uint16_t)bg_h_shift << endl;


                NES_LOG("PPU")
                    << std::format("shifted BGL: {:04X} BGH: {:04X}\n",
                                   bg_l_shift, bg_h_shift);

                if (scan_x == 256) {
                    inc_vert(v);
                }
                inc_hori(v);
            }

            if (scan_x == 257 && (ppumask.bg_show || ppumask.spr_show)) {
                set_hori(v, t);
            }

            if (scan_y == 261 && scan_x >= 280 && scan_x <= 304 &&
                (ppumask.bg_show || ppumask.spr_show)) {
                set_vert(v, t);
            }
        }

        if (scan_y == 239 && scan_x == 320) {
            if (!headless) {
                uint32_t *fbptr = fb_prim ? fb.data() : fb_sec.data();
                gui.draw_frame(fbptr);
            }
            fb_prim = !fb_prim;
            frame_count++;
        }

        if (scan_x == ntsc_x - 2 && scan_y == ntsc_y - 1 && scan_short &&
            (ppumask.bg_show || ppumask.spr_show)) {
            NES_LOG("PPU") << "Odd frame, jump from 339,261 to 0,0" << endl;
            // Jump directly from (339,261) to (0,0) on odd frames
            scan_short = !scan_short;
            scan_x = 0;
            scan_y = 0;
        } else {
            if (scan_x == ntsc_x-1 && scan_y == ntsc_y-1) {
                scan_short = !scan_short;
            }
            // Increment scanline/pixel counter
            if (scan_x == (ntsc_x - 1)) scan_y = (scan_y + 1) % ntsc_y;
            scan_x = (scan_x + 1) % ntsc_x;
        }

        cycles--;
    }
}

void PPU::cpu_write(uint16_t addr, uint8_t value) {
    NES_LOG("PPU") << std::format("cpu_write@{:04X} value={:02X}\n", addr,
                                  value);
    cpu_bus = value;
    switch (addr) {
    case 0x2000:  // PPUCTRL
        ppuctrl.value = value;
        t.nt_h = value & 0x1;
        t.nt_v = (value >> 1) & 0x1;
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
            t.h = value & 0x3F;
        else {
            t.l = value;
            v = t;
        }
        w = !w;
        NES_LOG("PPU") << "new v.addr: 0x" << v.addr << endl;
        break;
    case 0x2007:  // PPUDATA
        write(v.addr, value);
        if ((ppumask.bg_show || ppumask.spr_show) &&
            (scan_y == 261 || scan_y <= 239)) {
            inc_hori(v);
            inc_vert(v);
        } else {
            if (ppuctrl.v_incr) {
                v.addr += 0x20;
            } else {
                v.addr++;
            }
        }

        NES_LOG("PPU") << "new v.addr: 0x" << v.addr << endl;
        break;
    default: throw std::runtime_error("Invalid/unimplemented PPU write.");
    }
}

uint8_t PPU::cpu_read(uint16_t addr, bool passive) {
    if (!passive) {
        NES_LOG("PPU") << std::format(
            "cpu_read@{:04x} passive={} value={:02X}?\n", addr, passive,
            cpu_read(addr, true));
    }
    switch (addr) {
    case 0x2000:  // Write-only
        return cpu_bus;
    case 0x2001:  // Write-only
        return cpu_bus;
    case 0x2002:
        if (ppustatus.value == 0x80) {
            NES_LOG("PPU") << "hehe\n";
        }
        uint8_t out_status;
        out_status = ppustatus.value;
        if (!passive) {
            w = (bool)0;
            ppustatus.vblank = 0;
            cpu_bus = (ppustatus.value & 0xE0) | (cpu_bus & 0x1F);
        }
        return out_status;
    case 0x2003:  // Write-only
        return cpu_bus;
    case 0x2004:
        // TODO: Reads while rendering should expose internal OAM accesses
        if (!passive) {
            cpu_bus = oam[oamaddr];
        }
        return oam[oamaddr];
    case 0x2005:
    case 0x2006: return cpu_bus;
    case 0x2007:
        uint8_t ppudata_out;
        if (!passive) {
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
        } else {
            ppudata_out = ppudata_buf;
        }
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
    NES_LOG("PPU") << std::format("write {:02X} to {:04X}, mirror: {:d}\n",
                                  value, addr, (int)mirror);
    switch (addr) {
    case 0x0000 ... 0x1FFF: mapper->write_ppu(addr, value); break;
    case 0x2000 ... 0x23FF: vram[addr - 0x2000] = value; break;
    case 0x2400 ... 0x27FF:
        switch (mirror) {
        case map_hori: vram[addr - 0x2000] = value; break;
        case map_quad: vram[addr - 0x2000] = value; break;
        case map_vert: vram[addr - 0x2000] = value; break;
        case map_single: vram[addr - 0x2400] = value; break;
        }
        break;
    case 0x2800 ... 0x2BFF:
        switch (mirror) {
        case map_hori: vram[addr - 0x2400] = value; break;
        case map_single: vram[addr - 0x2800] = value; break;
        case map_vert: vram[addr - 0x2800] = value; break;
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
    case 0x3000 ... 0x3EFF: 
        write(addr-0x1000, value); break;
    case 0x3F00 ... 0x3FFF: pram[((addr - 0x3F00) % 0x20)] = value; break;
    default: throw std::runtime_error("Invalid/unimplemented PPU write");
    }
}

uint8_t PPU::read(uint16_t addr) {
    using namespace iNESv1::Mapper;
    NTMirror mirror = mapper->mirroring();
    NES_LOG("PPU") << std::format("read@{:04X}, mirror: {:d}\n", addr,
                                  (int)mirror);
    switch (addr) {
    case 0x0000 ... 0x1FFF: return mapper->read_ppu(addr); break;
    case 0x2000 ... 0x23FF: return vram[addr - 0x2000]; break;
    case 0x2400 ... 0x27FF:
        switch (mirror) {
        case map_hori: return vram[addr - 0x2000]; break;
        case map_quad: return vram[addr - 0x2000]; break;
        case map_vert: return vram[addr - 0x2000]; break;
        case map_single: return vram[addr - 0x2400]; break;
        }
        break;
    case 0x2800 ... 0x2BFF:
        switch (mirror) {
        case map_hori: return vram[addr - 0x2400]; break;
        case map_single: return vram[addr - 0x2800]; break;
        case map_vert: return vram[addr - 0x2800]; break;
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
    case 0x3000 ... 0x3EFF:
        return read(addr-0x1000); break;
    case 0x3F00 ... 0x3FFF: return pram[((addr - 0x3F00) % 0x20)]; break;
    default: break;
    }
    throw std::runtime_error("Invalid or unimplemented PPU read");
}
