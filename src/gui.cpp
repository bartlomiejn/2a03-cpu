#include <gui.h>
#include <ppu.h>
#include <cpu.h>

namespace GFX {

void DebugWindow::draw(NES::iNESv1::Mapper::Base *mapper) {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // Draw PPU state panel
    draw_ppu_state();

    // Draw CPU state panel
    draw_cpu_state();

    // Draw ROM info panel
    if (mapper)
        draw_rom_info(mapper);

    // Draw CHR viewer panel
    if (mapper)
        draw_chr_viewer(mapper);

    ImGui::Render();

    SDL_SetRenderDrawColor(ren, 30, 30, 30, 255);
    SDL_RenderClear(ren);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), ren);
    SDL_RenderPresent(ren);
}

void DebugWindow::draw_ppu_state() {
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(380, 550), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("PPU State")) {
        ImGui::End();
        return;
    }

    if (!ppu) {
        ImGui::Text("PPU not available");
        ImGui::End();
        return;
    }

    // Scanline position
    if (ImGui::CollapsingHeader("Scanline Position", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Pixel (X):    %3u / %u", ppu->scan_x, ppu->scan_x_end);
        ImGui::Text("Scanline (Y): %3u / %u", ppu->scan_y, ppu->scan_y_end);
        ImGui::Text("Short frame:  %s", ppu->scan_short ? "Yes" : "No");
        ImGui::Text("Primary FB:   %s", ppu->fb_prim ? "Yes" : "No");
    }

    ImGui::Separator();

    // PPUCTRL ($2000)
    if (ImGui::CollapsingHeader("PPUCTRL ($2000)", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Raw: $%02X", ppu->ppuctrl.value);
        ImGui::Text("Nametable select: %u ($%04X)",
                    ppu->ppuctrl.nt_xy_select,
                    0x2000 + ppu->ppuctrl.nt_xy_select * 0x400);
        ImGui::Text("VRAM increment:   %s",
                    ppu->ppuctrl.v_incr ? "+32 (down)" : "+1 (across)");
        ImGui::Text("Sprite PT addr:   $%04X",
                    ppu->ppuctrl.spr_pt_addr ? 0x1000 : 0x0000);
        ImGui::Text("BG PT addr:       $%04X",
                    ppu->ppuctrl.bg_pt_addr ? 0x1000 : 0x0000);
        ImGui::Text("Sprite size:      %s",
                    ppu->ppuctrl.spr_size ? "8x16" : "8x8");
        ImGui::Text("PPU master/slave: %s",
                    ppu->ppuctrl.ppu_master ? "Output" : "Input");
        ImGui::Text("VBlank NMI:       %s",
                    ppu->ppuctrl.vbl_nmi ? "Enabled" : "Disabled");
    }

    ImGui::Separator();

    // PPUMASK ($2001)
    if (ImGui::CollapsingHeader("PPUMASK ($2001)", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Raw: $%02X", ppu->ppumask.value);
        ImGui::Text("Grayscale:         %s", ppu->ppumask.grayscale ? "Yes" : "No");
        ImGui::Text("Show BG left 8px:  %s", ppu->ppumask.bg_show_left_8px ? "Yes" : "No");
        ImGui::Text("Show Spr left 8px: %s", ppu->ppumask.spr_show_left_8px ? "Yes" : "No");
        ImGui::Text("Show background:   %s", ppu->ppumask.bg_show ? "Yes" : "No");
        ImGui::Text("Show sprites:      %s", ppu->ppumask.spr_show ? "Yes" : "No");
        ImGui::Text("Emphasize R/G/B:   %c%c%c",
                    ppu->ppumask.r ? 'R' : '-',
                    ppu->ppumask.g ? 'G' : '-',
                    ppu->ppumask.b ? 'B' : '-');
    }

    ImGui::Separator();

    // PPUSTATUS ($2002)
    if (ImGui::CollapsingHeader("PPUSTATUS ($2002)", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Raw: $%02X", ppu->ppustatus.value);
        ImGui::Text("Sprite overflow: %s", ppu->ppustatus.spr_overflow ? "Yes" : "No");
        ImGui::Text("Sprite 0 hit:    %s", ppu->ppustatus.spr0_hit ? "Yes" : "No");
        ImGui::Text("VBlank:          %s", ppu->ppustatus.vblank ? "Yes" : "No");
    }

    ImGui::Separator();

    // Internal registers (v, t, x, w)
    if (ImGui::CollapsingHeader("Internal Registers", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("v (VRAM addr):  $%04X", ppu->v.addr);
        ImGui::Text("  Coarse X:     %2u", ppu->v.sc_x);
        ImGui::Text("  Coarse Y:     %2u", ppu->v.sc_y);
        ImGui::Text("  Nametable:    %u%u", ppu->v.nt_v, ppu->v.nt_h);
        ImGui::Text("  Fine Y:       %u", ppu->v.sc_fine_y);

        ImGui::Spacing();

        ImGui::Text("t (Temp addr):  $%04X", ppu->t.addr);
        ImGui::Text("  Coarse X:     %2u", ppu->t.sc_x);
        ImGui::Text("  Coarse Y:     %2u", ppu->t.sc_y);
        ImGui::Text("  Nametable:    %u%u", ppu->t.nt_v, ppu->t.nt_h);
        ImGui::Text("  Fine Y:       %u", ppu->t.sc_fine_y);

        ImGui::Spacing();

        ImGui::Text("x (Fine X):     %u", ppu->x.fine);
        ImGui::Text("w (Latch):      %s", ppu->w ? "Second write" : "First write");
    }

    ImGui::Separator();

    // OAM registers
    if (ImGui::CollapsingHeader("OAM Registers")) {
        ImGui::Text("OAMADDR:  $%02X", ppu->oamaddr & 0xFF);
        ImGui::Text("OAMDATA:  $%02X", ppu->oamdata);
    }

    ImGui::Separator();

    // Data buffers
    if (ImGui::CollapsingHeader("Data Buffers")) {
        ImGui::Text("PPUDATA buf: $%02X", ppu->ppudata_buf);
        ImGui::Text("CPU bus:     $%02X", ppu->cpu_bus);
        ImGui::Text("PPU bus:     $%04X", ppu->bus.addr);
    }

    ImGui::Separator();

    // Fetch state
    if (ImGui::CollapsingHeader("Fetch State")) {
        ImGui::Text("Nametable byte: $%02X", ppu->nt);
        ImGui::Text("Attribute byte: $%02X", ppu->at);
        ImGui::Text("BG shift low:   $%04X", ppu->bg_l_shift);
        ImGui::Text("BG shift high:  $%04X", ppu->bg_h_shift);
    }

    ImGui::End();
}

void DebugWindow::draw_cpu_state() {
    ImGui::SetNextWindowPos(ImVec2(10, 570), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(380, 350), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("CPU State")) {
        ImGui::End();
        return;
    }

    if (!cpu) {
        ImGui::Text("CPU not available");
        ImGui::End();
        return;
    }

    // Registers
    if (ImGui::CollapsingHeader("Registers", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("A:  $%02X (%3u)", cpu->A, cpu->A);
        ImGui::Text("X:  $%02X (%3u)", cpu->X, cpu->X);
        ImGui::Text("Y:  $%02X (%3u)", cpu->Y, cpu->Y);
    }

    ImGui::Separator();

    // Program Counter
    if (ImGui::CollapsingHeader("Program Counter", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("PC: $%04X", cpu->PC);
    }

    ImGui::Separator();

    // Stack Pointer
    if (ImGui::CollapsingHeader("Stack", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("S:  $%02X", cpu->S);
        ImGui::Text("Stack addr: $01%02X", cpu->S);
    }

    ImGui::Separator();

    // Status Register
    if (ImGui::CollapsingHeader("Status Register (P)", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Raw: $%02X", cpu->P.status);
        ImGui::Spacing();
        ImGui::Text("N V - B D I Z C");
        ImGui::Text("%c %c %c %c %c %c %c %c",
                    cpu->P.N ? '1' : '0',
                    cpu->P.V ? '1' : '0',
                    '-',
                    (cpu->P.B & 0x2) ? '1' : '0',
                    cpu->P.D ? '1' : '0',
                    cpu->P.I ? '1' : '0',
                    cpu->P.Z ? '1' : '0',
                    cpu->P.C ? '1' : '0');
        ImGui::Spacing();
        ImGui::Text("N (Negative):  %s", cpu->P.N ? "Set" : "Clear");
        ImGui::Text("V (Overflow):  %s", cpu->P.V ? "Set" : "Clear");
        ImGui::Text("B (Break):     %u", cpu->P.B);
        ImGui::Text("D (Decimal):   %s", cpu->P.D ? "Set" : "Clear");
        ImGui::Text("I (IRQ Dis.):  %s", cpu->P.I ? "Set" : "Clear");
        ImGui::Text("Z (Zero):      %s", cpu->P.Z ? "Set" : "Clear");
        ImGui::Text("C (Carry):     %s", cpu->P.C ? "Set" : "Clear");
    }

    ImGui::Separator();

    // Interrupt State
    if (ImGui::CollapsingHeader("Interrupts", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("IRQ line: %s", cpu->IRQ ? "Asserted" : "Clear");
        ImGui::Text("NMI line: %s", cpu->NMI ? "Asserted" : "Clear");
    }

    ImGui::Separator();

    // Execution State
    if (ImGui::CollapsingHeader("Execution", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Current opcode: $%02X", cpu->opcode);
        ImGui::Text("Cycle count:    %u", cpu->cycles);
    }

    ImGui::End();
}

void DebugWindow::render_chr_table(std::vector<uint32_t> &fb,
                                    NES::iNESv1::Mapper::Base *mapper,
                                    unsigned int base_addr) {
    const unsigned int tile_size = 16;  // 8 bytes plane 0 + 8 bytes plane 1
    const unsigned int tiles_per_row = 16;
    const unsigned int total_tiles = 256;

    size_t chr_rom_size = mapper->cartridge.chr_rom.size();

    for (unsigned int tile_idx = 0; tile_idx < total_tiles; tile_idx++) {
        size_t tile_start = base_addr + tile_idx * tile_size;
        if (tile_start + tile_size > chr_rom_size)
            break;

        for (int y = 0; y < 8; y++) {
            uint8_t p0 = mapper->cartridge.chr_rom[tile_start + y];
            uint8_t p1 = mapper->cartridge.chr_rom[tile_start + y + 8];
            for (int x = 0; x < 8; x++) {
                bool b0 = (p0 >> (7 - x)) & 1;
                bool b1 = (p1 >> (7 - x)) & 1;
                uint8_t c_idx = (b1 << 1) | b0;

                uint32_t c;
                switch (c_idx) {
                case 0:  c = 0x000000FF; break;
                case 1:  c = 0x555555FF; break;
                case 2:  c = 0xAAAAAAFF; break;
                case 3:  c = 0xFFFFFFFF; break;
                }

                unsigned int px_x = (tile_idx % tiles_per_row) * 8 + x;
                unsigned int px_y = (tile_idx / tiles_per_row) * 8 + y;
                int idx = px_y * chr_fb_size + px_x;
                fb[idx] = c;
            }
        }
    }
}

void DebugWindow::draw_rom_info(NES::iNESv1::Mapper::Base *mapper) {
    ImGui::SetNextWindowPos(ImVec2(400, 370), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 220), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("ROM Info")) {
        ImGui::End();
        return;
    }

    const auto &cart = mapper->cartridge;
    const auto &hdr = cart.header;

    // Header info
    if (ImGui::CollapsingHeader("Header", ImGuiTreeNodeFlags_DefaultOpen)) {
        uint8_t mapper_num = (hdr.flags_7.nib_h << 4) | hdr.flags_6.nib_l;
        ImGui::Text("Mapper:       %u", mapper_num);
        ImGui::Text("PRG ROM banks: %u (x16KB)", hdr.prg_rom_banks);
        ImGui::Text("CHR ROM banks: %u (x8KB)", hdr.chr_rom_banks);
        ImGui::Text("PRG RAM size:  %u bytes", hdr.prg_ram_banks);
        ImGui::Text("Mirroring:    %s", hdr.flags_6.mirror ? "Vertical" : "Horizontal");
        ImGui::Text("Battery:      %s", hdr.flags_6.prg_ram ? "Yes" : "No");
        ImGui::Text("Trainer:      %s", hdr.flags_6.has_trainer ? "Yes" : "No");
        ImGui::Text("iNESv2:       %s",
                    hdr.flags_7.ines_v2 == 2 ? "Yes" : "No");
        ImGui::Text("System (v1):  %s", hdr.flags_9.tv_sys ? "PAL" : "NTSC");
    }

    ImGui::Separator();

    // Actual data sizes
    if (ImGui::CollapsingHeader("Data Sizes", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Trainer:  %zu bytes", cart.trainer.size());
        ImGui::Text("PRG ROM:  %zu bytes (%zu KB)", cart.prg_rom.size(), cart.prg_rom.size() / 1024);
        ImGui::Text("CHR ROM:  %zu bytes (%zu KB)", cart.chr_rom.size(), cart.chr_rom.size() / 1024);
        ImGui::Text("PRG RAM:  %zu bytes (%zu KB)", cart.prg_ram.size(), cart.prg_ram.size() / 1024);
    }

    ImGui::End();
}

void DebugWindow::draw_chr_viewer(NES::iNESv1::Mapper::Base *mapper) {
    ImGui::SetNextWindowPos(ImVec2(400, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 350), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("CHR ROM Viewer")) {
        ImGui::End();
        return;
    }

    // Render both CHR tables
    render_chr_table(chr_fb_0, mapper, 0x0000);
    render_chr_table(chr_fb_1, mapper, 0x1000);

    // Update textures
    SDL_UpdateTexture(chr_tex_0, NULL, chr_fb_0.data(),
                      chr_fb_size * sizeof(uint32_t));
    SDL_UpdateTexture(chr_tex_1, NULL, chr_fb_1.data(),
                      chr_fb_size * sizeof(uint32_t));

    // Display CHR table 0 ($0000-$0FFF)
    ImGui::Text("Pattern Table 0 ($0000-$0FFF)");
    ImGui::Image((ImTextureID)(intptr_t)chr_tex_0,
                 ImVec2((float)chr_fb_size, (float)chr_fb_size));

    ImGui::Spacing();

    // Display CHR table 1 ($1000-$1FFF)
    ImGui::Text("Pattern Table 1 ($1000-$1FFF)");
    ImGui::Image((ImTextureID)(intptr_t)chr_tex_1,
                 ImVec2((float)chr_fb_size, (float)chr_fb_size));

    ImGui::End();
}

}  // namespace GFX
