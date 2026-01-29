#include <gui.h>
#include <ppu.h>

namespace GFX {

void DebugWindow::draw(NES::iNESv1::Mapper::Base *mapper) {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // Draw PPU state panel
    draw_ppu_state();

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

void DebugWindow::draw_chr_viewer(NES::iNESv1::Mapper::Base *mapper) {
    ImGui::SetNextWindowPos(ImVec2(400, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(380, 300), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("CHR ROM Viewer")) {
        ImGui::End();
        return;
    }

    ImGui::InputText("CHR start address", chr_addr_input,
                     IM_ARRAYSIZE(chr_addr_input),
                     ImGuiInputTextFlags_CharsHexadecimal);
    if (chr_addr_input[0]) {
        std::stringstream ss;
        ss << std::hex << chr_addr_input;
        ss >> chr_addr;
    }

    // Render CHR tiles to framebuffer
    const unsigned int plane_size = 8;
    const unsigned int tile_size = plane_size * 2;
    const unsigned int tiles_on_line = chr_fb_x / 8;
    const unsigned int tiles_count_y = chr_fb_y / 8;

    size_t chr_rom_size = mapper->cartridge.chr_rom.size();
    size_t tile_idx = 0;
    for (size_t tile_start = 0x0; tile_start < chr_rom_size;
         tile_start += tile_size) {
        for (int y = 0; y < 8; y++) {
            uint8_t p0 = mapper->cartridge.chr_rom[tile_start + y];
            uint8_t p1 = mapper->cartridge.chr_rom[tile_start + y + 8];
            for (int x = 0; x < 8; x++) {
                bool b0 = (p0 >> (7 - x)) & 1;
                bool b1 = (p1 >> (7 - x)) & 1;
                uint8_t c_idx = (b1 << 1) | b0;

                uint32_t c;
                switch (c_idx) {
                case 0:
                    c = 0x000000FF;
                    break;
                case 1:
                    c = 0x555555FF;
                    break;
                case 2:
                    c = 0xAAAAAAFF;
                    break;
                case 3:
                    c = 0xFFFFFFFF;
                    break;
                }
                unsigned int c_x = (tile_idx % tiles_on_line) * 8 + x;
                unsigned int c_y = (tile_idx / tiles_on_line) * 8 + y;
                int idx = c_y * chr_fb_x + c_x;
                if (idx < chr_fb_x * chr_fb_y)
                    chr_fb[idx] = c;
            }
        }
        tile_idx++;
        if (tile_idx >= tiles_on_line * tiles_count_y)
            break;
    }

    // Update texture and display as image
    SDL_UpdateTexture(chr_tex, NULL, chr_fb.data(), chr_fb_x * sizeof(uint32_t));

    // Display the CHR texture as an ImGui image
    ImGui::Image((ImTextureID)(intptr_t)chr_tex, ImVec2((float)chr_fb_x, (float)chr_fb_y));

    ImGui::End();
}

}  // namespace GFX
