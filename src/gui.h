#ifndef INC_2A03_GUI_H
#define INC_2A03_GUI_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <mapper.h>

#include <atomic>
#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>

// Forward declaration
namespace NES { class PPU; }

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

class DebugWindow {
public:
    SDL_Window *wnd;
    SDL_Renderer *ren;
    SDL_Texture *chr_tex;

    std::vector<uint32_t> chr_fb;
    int chr_fb_x, chr_fb_y;

    char chr_addr_input[8] = "0";
    unsigned int chr_addr = 0;

    NES::PPU *ppu = nullptr;

    int wnd_w, wnd_h;

    DebugWindow(int chr_fb_x, int chr_fb_y, int wnd_w = 800, int wnd_h = 600)
        : chr_fb_x(chr_fb_x), chr_fb_y(chr_fb_y), wnd_w(wnd_w), wnd_h(wnd_h) {
        wnd = SDL_CreateWindow("Debugger", SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED, wnd_w, wnd_h,
                               SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
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

        chr_tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888,
                                    SDL_TEXTUREACCESS_STREAMING, chr_fb_x,
                                    chr_fb_y);
        if (!chr_tex) {
            std::cerr << "SDL_CreateTexture error: " << SDL_GetError()
                      << std::endl;
            SDL_DestroyRenderer(ren);
            SDL_DestroyWindow(wnd);
            SDL_Quit();
            throw std::runtime_error("SDL_CreateTexture error");
        }

        ImGui_ImplSDL2_InitForSDLRenderer(wnd, ren);
        ImGui_ImplSDLRenderer2_Init(ren);

        chr_fb.resize(chr_fb_x * chr_fb_y);
        memset(chr_fb.data(), 0, sizeof(uint32_t) * chr_fb_x * chr_fb_y);
    }

    ~DebugWindow() {
        if (chr_tex)  SDL_DestroyTexture(chr_tex);
        if (ren)      SDL_DestroyRenderer(ren);
        if (wnd)      SDL_DestroyWindow(wnd);
    }

    void draw(NES::iNESv1::Mapper::Base *mapper);

private:
    void draw_ppu_state();
    void draw_chr_viewer(NES::iNESv1::Mapper::Base *mapper);
};

class GUI {
   public:
    MainWindow *main;
    DebugWindow *debug;
    NES::iNESv1::Mapper::Base *mapper;
    NES::PPU *ppu = nullptr;

    std::atomic<bool> main_fb_ready = false;
    uint32_t *main_fb = nullptr;

    const int fb_x, fb_y;
    const std::string main_font_name = "Inter-VariableFont.ttf";
    const float main_font_size = 18.0f;

    bool stop = false;

    GUI(int fb_x, int fb_y) : fb_x(fb_x), fb_y(fb_y) {};

    ~GUI() {
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        if (debug) delete debug;
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
        debug = new DebugWindow(fb_x, fb_y);
    }

    void enter_runloop() {
        main->draw_frame(nullptr);

        // Pass PPU reference to the debug window
        if (debug && ppu)
            debug->ppu = ppu;

        while (!stop) {
            if (handle_events()) break;

            if (main_fb_ready) {
                main->draw_frame(main_fb);
                main_fb_ready = false;
            }

            if (debug)
                debug->draw(mapper);
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
