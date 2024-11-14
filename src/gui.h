#ifndef INC_2A03_GUI_H
#define INC_2A03_GUI_H

#include <sstream>
#include <atomic>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

namespace GFX {

class MainWindow {
public:
    SDL_Window *wnd;
    SDL_Renderer *ren;
    SDL_Texture *tex;

    int fb_x, fb_y;

    MainWindow(int fb_x, int fb_y) : fb_x(fb_x), fb_y(fb_y) {
        wnd = SDL_CreateWindow("2A03", SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED, fb_x*2, fb_y*2,
                             SDL_WINDOW_SHOWN);
        if (!wnd) {
            std::cerr << "SDL_CreateWindow error: " << SDL_GetError()
                      << std::endl;
            SDL_Quit();
            throw std::runtime_error("SDL_CreateWindow error");
        }

        ren = SDL_CreateRenderer(wnd, -1, SDL_RENDERER_ACCELERATED);
        if (!ren) {
            std::cerr << "SDL_CreateRenderer error: " << SDL_GetError()
                      << std::endl;
            SDL_DestroyWindow(wnd);
            SDL_Quit();
            throw std::runtime_error("SDL_CreateRenderer error");
        }

        tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888,
                                SDL_TEXTUREACCESS_STREAMING, fb_x,
                                fb_y);
        if (!tex) {
            std::cerr << "SDL_CreateTexture error: " << SDL_GetError()
                      << std::endl;
            SDL_DestroyRenderer(ren);
            SDL_DestroyWindow(wnd);
            SDL_Quit();
            throw std::runtime_error("SDL_CreateTexture error");
        }

        // TODO: Can't init it twice. Used only for CHR window for now so leave it
        // ImGui_ImplSDL2_InitForSDLRenderer(wnd, ren);
        // ImGui_ImplSDLRenderer2_Init(ren); 
    }

    ~MainWindow() {
        if (tex)   SDL_DestroyTexture(tex);
        if (ren)   SDL_DestroyRenderer(ren);
        if (wnd)   SDL_DestroyWindow(wnd);
    }

    void draw_frame(uint32_t *fb) {
        if (fb) 
            SDL_UpdateTexture(tex, NULL, fb, fb_x * sizeof(uint32_t));
        SDL_Rect dest = { 0, 0, fb_x*2, fb_y*2 };
        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, tex, NULL, &dest);
        SDL_RenderPresent(ren);
    }
};

class ChrViewerWindow {
public:
    SDL_Window *wnd;
    SDL_Renderer *ren;
    SDL_Texture *tex;

    std::vector<uint32_t> fb;
    int fb_x, fb_y;

    char chr_addr_input[8] = "0";
    unsigned int chr_addr = 0;

    SDL_Rect dest_rect;

    ChrViewerWindow(int fb_x, int fb_y) : fb_x(fb_x), fb_y(fb_y) {
        wnd = SDL_CreateWindow("CHR ROM Viewer", SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED, fb_x*2, fb_y*2,
                               SDL_WINDOW_SHOWN);
        if (!wnd) {
            std::cerr << "SDL_CreateWindow error: " << SDL_GetError()
                      << std::endl;
            SDL_Quit();
            throw std::runtime_error("SDL_CreateWindow error");
        }

        ren = SDL_CreateRenderer(wnd, -1, SDL_RENDERER_ACCELERATED);
        if (!ren) {
            std::cerr << "SDL_CreateRenderer error: " << SDL_GetError()
                      << std::endl;
            SDL_DestroyWindow(wnd);
            SDL_Quit();
            throw std::runtime_error("SDL_CreateRenderer error");
        }

        tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888,
                                SDL_TEXTUREACCESS_STREAMING, fb_x,
                                fb_y);
        if (!tex) {
            std::cerr << "SDL_CreateTexture error: " << SDL_GetError()
                      << std::endl;
            SDL_DestroyRenderer(ren);
            SDL_DestroyWindow(wnd);
            SDL_Quit();
            throw std::runtime_error("SDL_CreateTexture error");
        }

        ImGui_ImplSDL2_InitForSDLRenderer(wnd, ren);
        ImGui_ImplSDLRenderer2_Init(ren); 

        fb.reserve(fb_x*fb_y);
        memset(fb.data(), 0, sizeof(uint32_t)*fb_x*fb_y);

        dest_rect = { 0, 0, fb_x*2, fb_y*2};
    }

    ~ChrViewerWindow() {
        if (tex)    SDL_DestroyTexture(tex);
        if (ren)    SDL_DestroyRenderer(ren);
        if (wnd)    SDL_DestroyWindow(wnd);
    }

    void draw(NES::iNESv1::Mapper::Base &mapper) {
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Overlay");
        ImGui::InputText("CHR start address", chr_addr_input, 
                         IM_ARRAYSIZE(chr_addr_input),
                         ImGuiInputTextFlags_CharsHexadecimal);
        if (chr_addr_input[0]) {
            std::stringstream ss;
            ss << std::hex << chr_addr_input;
            ss >> chr_addr;
        }
        ImGui::End();

        const unsigned int plane_size = 8; // TODO: For now only handle 8x8 
        const unsigned int tile_size = plane_size*2;
        const unsigned int tiles_on_line = fb_x / 8;
        const unsigned int tiles_count_y = fb_y / 8;

        size_t chr_rom_size = mapper.cartridge.chr_rom.size();
        size_t tile_idx = 0;
        for (size_t tile_start = 0x0; tile_start < chr_rom_size; 
             tile_start += tile_size) {
            for (int y = 0; y < 8; y++) {
                uint8_t p0 = mapper.cartridge.chr_rom[tile_start+y];
                uint8_t p1 = mapper.cartridge.chr_rom[tile_start+y+8];
                for (int x = 0; x < 8; x++) {
                    bool b0 = (p0 >> (7 - x)) & 1;
                    bool b1 = (p1 >> (7 - x)) & 1;
                    uint8_t c_idx = (b1 << 1) | b0;

                    // TODO: Actually map colors to palette values later
                    uint32_t c;
                    switch (c_idx) {
                    case 0:
                        c = 0x000000FF; break;
                    case 1:
                        c = 0x555555FF; break;
                    case 2:
                        c = 0xAAAAAAFF; break;
                    case 3:
                        c = 0xFFFFFFFF; break;
                    }
                    unsigned int c_x = (tile_idx % tiles_on_line)*8 + x;
                    unsigned int c_y = (tile_idx / tiles_on_line)*8 + y;
                    int idx = c_y * fb_x + c_x;
                    if (idx < fb_x*fb_y)
                        fb[idx] = c;
                }
            }
            tile_idx++;
            if (tile_idx >= tiles_on_line * tiles_count_y)
                break;
        }

        ImGui::Render();

        SDL_UpdateTexture(tex, NULL, fb.data(), fb_x * sizeof(uint32_t));
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, tex, nullptr, &dest_rect);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), ren);
        SDL_RenderPresent(ren);
    }
};

class GUI {
   public:
    MainWindow *main;
    ChrViewerWindow *chr_viewer;
    NES::iNESv1::Mapper::Base *mapper;

    std::atomic<bool> main_fb_ready = false;
    uint32_t *main_fb = nullptr;

    const int fb_x, fb_y;
    const std::string main_font_name = "Inter-VariableFont.ttf";
    const float main_font_size = 18.0f; 

    GUI(int fb_x, int fb_y) : fb_x(fb_x), fb_y(fb_y) {};

    ~GUI() {
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        if (chr_viewer) delete chr_viewer;
        if (main) delete main;

        SDL_Quit();
    };

    void setup() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
            std::cerr << "SDL_Init error: " << SDL_GetError() << std::endl;
            throw std::runtime_error("SDL_Init error");
        } 

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.Fonts->AddFontFromFileTTF(main_font_name.c_str(), main_font_size);
        ImGui::StyleColorsDark();

        main = new MainWindow(fb_x, fb_y);
        chr_viewer = new ChrViewerWindow(fb_x, fb_y);
    }

    void enter_runloop() {
        main->draw_frame(nullptr);

        while (true) {
            // TODO: Guard FB access
            
            if (handle_events()) break;

            if (main_fb_ready) {
                main->draw_frame(main_fb);
                main_fb_ready = false;
            }

            if (chr_viewer)
                chr_viewer->draw(*mapper);
        }
    }

    void draw_frame(uint32_t *fb) {
        main_fb = fb;
        main_fb_ready = true;
    }

    static bool handle_events() {
        bool quit = false;
        SDL_Event event;
        SDL_PollEvent(&event);

        ImGui_ImplSDL2_ProcessEvent(&event);

        if (event.type == SDL_WINDOWEVENT 
            && event.window.event == SDL_WINDOWEVENT_CLOSE) 
            // TODO: Close only when event comes from main window
            quit = true;
        if (event.type == SDL_QUIT)
            quit = true;

        return quit;
    }
};

}  // namespace GFX

#endif  // INC_2A03_GUI_H
