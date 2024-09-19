#include <ppu.h>

using namespace NES;

PPU::PPU() {}

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
}

void PPU::step() {}

void PPU::write(uint16_t addr, uint8_t value) {
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

uint8_t PPU::read(uint16_t addr) {
    switch (addr) {
        case 0x2002:
            return ppustatus.value;
        case 0x2004:
            // TODO: Reads while rendering should expose internal OAM accesses
            return oam[oamaddr];
        case 0x2007:
            uint8_t ppuval;
            ppuval = vram[ppuaddr16];
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
        case 0x0 ... 0x3F1F:
            vram[ppuaddr16] = value;
            break;
        case 0x3F20 ... 0x3FFF:
            throw std::runtime_error("Pallete mirroring unimplemented");
    }
    if (ppuctrl.vram_addr_incr) ppuaddr16++;
}
