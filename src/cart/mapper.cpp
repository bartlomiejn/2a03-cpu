#include <cart/mapper.h>

#include <iostream>

using namespace NES::iNESv1;

Mapper::Base *Mapper::mapper(NES::iNESv1::Cartridge &cartridge) {
    uint8_t id = (cartridge.header.flags_7.nib_h << 4) |
                 (cartridge.header.flags_6.nib_l);
    switch (id) {
    case Mapper::type_NROM:
        std::cerr << "Mapper type NROM" << std::endl;
        return new NES::iNESv1::Mapper::NROM(cartridge);
    case Mapper::type_MMC1:
        std::cerr << "Mapper type MMC1" << std::endl;
        return new NES::iNESv1::Mapper::MMC1(cartridge);
    default:
        std::cerr << "Unimplemented mapper type: " << std::hex << id << "."
                  << std::endl;
        throw UnimplementedType();
    }
}

// NROM

Mapper::NROM::NROM(Cartridge &cartridge) : Mapper::Base(cartridge) {}

uint8_t Mapper::NROM::read_prg(uint16_t addr) {
    switch (addr) {
    case 0x6000 ... 0x7FFF: 
        if ((size_t)(addr - 0x6000) >= cartridge.prg_ram.size()) {
            std::cerr << "PRG read exceeds PRG RAM size, addr: $" << std::hex 
                << addr - 0x6000 << " prg_ram size: 0x" << cartridge.prg_ram.size() << std::endl;
            return 0x0;
        } else {
            return cartridge.prg_ram[addr - 0x6000];
        }
    case 0x8000 ... 0xBFFF:
        // Low 16KB PRG ROM 
        if ((size_t)(addr - 0x8000) >= cartridge.prg_rom.size()) {
            std::cerr << "PRG read exceeds PRG RAM size, addr: " << std::hex 
                << addr - 0x8000 << std::endl;
            return 0x0;
        } else {
            return cartridge.prg_rom[addr - 0x8000];
        }
    case 0xC000 ... 0xFFFF:
        // High 16KB PRG ROM, or mirrored low if 16KB
        uint16_t base; 
        if (cartridge.header.prg_rom_banks == 1)
            base = 0xC000;
        else
            base = 0x8000;

        if ((size_t)(addr - base) >= cartridge.prg_rom.size()) {
            std::cerr << "PRG read exceeds PRG ROM size, addr: " << std::hex 
                << addr - base << std::endl;
            return 0x0;
        } else {
            return cartridge.prg_rom[addr - base];
        }

    default:
        std::cout << "Invalid NROM Mapper memory access: $"
                  << static_cast<int>(addr) << std::endl;
        throw std::runtime_error("Invalid PRG memory access.");
    }
}

void Mapper::NROM::write_prg(uint16_t addr, uint8_t val) {
    switch (addr) {
    case 0x6000 ... 0x7FFF: cartridge.prg_ram[addr - 0x6000] = val; break;
    default: throw std::runtime_error("Invalid PRG write addr");
    }
}

Mapper::NTMirror Mapper::NROM::mirroring() {
    if (cartridge.header.flags_6.ignore_mctrl)
        return Mapper::NTMirror::map_quad;
    else if (cartridge.header.flags_6.mirror)
        return Mapper::NTMirror::map_vert;
    else
        return Mapper::NTMirror::map_hori;
}

uint8_t Mapper::NROM::read_ppu(uint16_t addr) {
    switch (addr) {
    case 0x0000 ... 0x1FFF: 
        if (addr >= cartridge.chr_rom.size()) {
            std::cerr << "PPU read exceeds CHR ROM size, addr: " << std::hex 
                << addr << std::endl;
            return 0x0;
        } else {
            return cartridge.chr_rom[addr]; 
        }
        break;
    default: throw std::runtime_error("Invalid CHR read addr");
    }
}

void Mapper::NROM::write_ppu(uint16_t addr, uint8_t val) {
    throw std::runtime_error("Write CHR unimplemented");
}

// MMC1

Mapper::MMC1::MMC1(Cartridge &cartridge)
    : Mapper::Base(cartridge),
      shift_reg(0),
      shift_count(0),
      prg_bank_swap(swap_l_prg_bank),
      prg_bank_sz(size_16k),
      chr_bank_sz(CHRBankSize(0)),
      prg_bank(0),
      wram_enable(0) {}

static bool is_low_bank(uint16_t addr) {
    return addr >= 0x8000 && addr <= 0xBFFF;
}

uint8_t Mapper::MMC1::read_prg(uint16_t addr) {
    switch (addr) {
    case 0x6000 ... 0x7FFF:
        // TODO: PRG RAM bankswitching?
        return cartridge.prg_ram[addr - 0x6000];
    case 0x8000 ... 0xFFFF:
        if (prg_bank_sz == size_32k)
            return read_32k_prg_bank(addr);
        else if (is_low_bank(addr))
            return read_l_16k_prg_bank(addr);
        else
            return read_h_16k_prg_bank(addr);
    default:
        std::cout << "Invalid MMC1 Mapper memory access: $"
                  << static_cast<int>(addr) << std::endl;
        return 0x0;
    }
}

void Mapper::MMC1::write_prg(uint16_t addr, uint8_t val) {
    switch (addr) {
    case 0x6000 ... 0x7FFF: cartridge.prg_ram[addr - 0x6000] = val; break;
    case 0x8000 ... 0xFFFF:
        if ((val & 0x80) == 0)
            set_shift_reg(addr, val);
        else
            reset_shift_reg();
        // TODO: Should we reset bank state as well?
        break;
    default:
        std::cerr << "Invalid address passed to MMC1: $" << std::hex
                  << static_cast<int>(addr) << "." << std::endl;
        throw InvalidAddress();
    }
}

uint8_t Mapper::MMC1::read_ppu(uint16_t addr) {
    throw std::runtime_error("Read CHR unimplemented");
}

void Mapper::MMC1::write_ppu(uint16_t addr, uint8_t val) {
    throw std::runtime_error("Write CHR unimplemented");
}

void Mapper::MMC1::set_shift_reg(uint16_t addr, uint8_t val) {
    shift_reg |= (val & 0b1) < shift_count;
    shift_count++;
    if (shift_count == 5) {
        set_reg(reg_number(addr));
        reset_shift_reg();
    }
}

void Mapper::MMC1::reset_shift_reg() {
    shift_reg = 0x0;
    shift_count = 0x0;
}

int Mapper::MMC1::reg_number(uint16_t addr) {
    switch (addr) {
    case 0x8000 ... 0x9FFF: return reg_main_ctrl;
    case 0xA000 ... 0xBFFF: return reg_l_chr_rom;
    case 0xC000 ... 0xDFFF: return reg_h_chr_rom;
    case 0xE000 ... 0xFFFF: return reg_prg_bank;
    default:
        std::cerr << "Invalid address passed to MMC1: $" << std::hex
                  << static_cast<int>(addr) << "." << std::endl;
        throw InvalidAddress();
    }
}

void Mapper::MMC1::set_reg(int reg_number) {
    switch (reg_number) {
    case reg_main_ctrl: set_main_ctrl_reg(shift_reg); break;
    case reg_l_chr_rom:
        // TODO: L CHR ROM register.
        std::cerr << "Unimplemented L CHR ROM Bank Register "
                     "access."
                  << std::endl;
        break;
    case reg_h_chr_rom:
        // TODO: R CHR ROM register.
        std::cerr << "Unimplemented R CHR ROM Bank Register "
                     "access."
                  << std::endl;
        break;
    case reg_prg_bank: set_prg_bank_reg(shift_reg); break;
    default: break;
    }
}

void Mapper::MMC1::set_main_ctrl_reg(uint8_t value) {
    // TODO: PPU mirroring type.
    switch (value & 0b11) {
    case 0:
    case 1:
    case 2:
    case 3:
    default: break;
    }
    prg_bank_swap = PRGBankSwap((value >> 2) & 0b1);
    prg_bank_sz = PRGBankSize((value >> 3) & 0b1);
    chr_bank_sz = CHRBankSize((value >> 4) & 0b1);
}

void Mapper::MMC1::set_prg_bank_reg(uint8_t value) {
    prg_bank = (uint8_t)(value & 0b1111);
    wram_enable = (bool)((value & 0b10000) >> 4);
}

uint8_t Mapper::MMC1::read_32k_prg_bank(uint16_t addr) {
    uint16_t prg_offset = addr - (uint16_t)0x8000;
    // Only upper 3 bits are used when determining a 32k PRG bank
    uint32_t abs_addr = (prg_bank >> 1) * prg_rom_page_sz + prg_offset;
    return cartridge.prg_rom[abs_addr];
}

uint8_t Mapper::MMC1::read_l_16k_prg_bank(uint16_t addr) {
    uint16_t prg_offset = addr - (uint16_t)0x8000;
    if (prg_bank_swap == swap_h_prg_bank)
        return cartridge.prg_rom[prg_offset];
    else  // Low bank is swappable.
    {
        uint32_t abs_addr = prg_bank * prg_rom_page_sz + prg_offset;
        return cartridge.prg_rom[abs_addr];
    }
}

uint8_t Mapper::MMC1::read_h_16k_prg_bank(uint16_t addr) {
    uint16_t prg_offset = addr - (uint16_t)0xC000;
    if (prg_bank_swap == swap_l_prg_bank) {
        uint32_t abs_addr =
            (cartridge.header.prg_rom_banks - 1) * prg_rom_page_sz + prg_offset;
        return cartridge.prg_rom[abs_addr];
    } else  // High bank is swappable.
    {
        uint32_t abs_addr = prg_bank * prg_rom_page_sz + prg_offset;
        return cartridge.prg_rom[abs_addr];
    }
}

Mapper::NTMirror Mapper::MMC1::mirroring() {
    if (cartridge.header.flags_6.ignore_mctrl)
        return Mapper::NTMirror::map_quad;
    else if (cartridge.header.flags_6.mirror)
        return Mapper::NTMirror::map_vert;
    else
        return Mapper::NTMirror::map_hori;
}
