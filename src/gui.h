#ifndef INC_2A03_GUI_H
#define INC_2A03_GUI_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <controller.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <log.h>
#include <mapper.h>

#include <atomic>
#include <format>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

// Forward declaration
namespace NES { class PPU; class CPU; }

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
    SDL_Texture *chr_tex_0;  // CHR table at 0x0000
    SDL_Texture *chr_tex_1;  // CHR table at 0x1000

    std::vector<uint32_t> chr_fb_0;  // Framebuffer for 0x0000
    std::vector<uint32_t> chr_fb_1;  // Framebuffer for 0x1000
    static constexpr int chr_fb_size = 128;  // 16 tiles × 8 pixels

    NES::PPU *ppu = nullptr;
    NES::CPU *cpu = nullptr;

    int wnd_w, wnd_h;

    DebugWindow(int wnd_w = 800, int wnd_h = 600)
        : wnd_w(wnd_w), wnd_h(wnd_h) {
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

        chr_tex_0 = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888,
                                      SDL_TEXTUREACCESS_STREAMING,
                                      chr_fb_size, chr_fb_size);
        chr_tex_1 = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888,
                                      SDL_TEXTUREACCESS_STREAMING,
                                      chr_fb_size, chr_fb_size);
        if (!chr_tex_0 || !chr_tex_1) {
            std::cerr << "SDL_CreateTexture error: " << SDL_GetError()
                      << std::endl;
            SDL_DestroyRenderer(ren);
            SDL_DestroyWindow(wnd);
            SDL_Quit();
            throw std::runtime_error("SDL_CreateTexture error");
        }

        ImGui_ImplSDL2_InitForSDLRenderer(wnd, ren);
        ImGui_ImplSDLRenderer2_Init(ren);

        chr_fb_0.resize(chr_fb_size * chr_fb_size);
        chr_fb_1.resize(chr_fb_size * chr_fb_size);
        memset(chr_fb_0.data(), 0, sizeof(uint32_t) * chr_fb_size * chr_fb_size);
        memset(chr_fb_1.data(), 0, sizeof(uint32_t) * chr_fb_size * chr_fb_size);
    }

    ~DebugWindow() {
        if (chr_tex_0) SDL_DestroyTexture(chr_tex_0);
        if (chr_tex_1) SDL_DestroyTexture(chr_tex_1);
        if (ren)       SDL_DestroyRenderer(ren);
        if (wnd)       SDL_DestroyWindow(wnd);
    }

    void draw(NES::iNESv1::Mapper::Base *mapper);

private:
    void draw_ppu_state();
    void draw_cpu_state();
    void draw_rom_info(NES::iNESv1::Mapper::Base *mapper);
    void draw_chr_viewer(NES::iNESv1::Mapper::Base *mapper);
    void render_chr_table(std::vector<uint32_t> &fb,
                          NES::iNESv1::Mapper::Base *mapper,
                          unsigned int base_addr);
};

class GUI {
   public:
    MainWindow *main;
    DebugWindow *debug;
    NES::iNESv1::Mapper::Base *mapper;
    NES::PPU *ppu = nullptr;
    NES::CPU *cpu = nullptr;
    NES::Controller *controller1 = nullptr;

    std::atomic<bool> main_fb_ready = false;
    uint32_t *main_fb = nullptr;

    const int fb_x, fb_y;
    const std::string main_font_name = "Inter-VariableFont.ttf";
    const float main_font_size = 18.0f;

    bool stop = false;

    GUI(int fb_x, int fb_y) : fb_x(fb_x), fb_y(fb_y) {};

    ~GUI() {
        if (debug) delete debug;
        if (main) {
            ImGui_ImplSDLRenderer2_Shutdown();
            ImGui_ImplSDL2_Shutdown();
            ImGui::DestroyContext();
            delete main;
            SDL_Quit();
        }
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
        debug = new DebugWindow();
    }

    void enter_runloop() {
        main->draw_frame(nullptr);

        // Pass PPU and CPU references to the debug window
        if (debug) {
            if (ppu) debug->ppu = ppu;
            if (cpu) debug->cpu = cpu;
        }

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

    bool handle_events() {
        bool quit = false;
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);

            if (event.type == SDL_WINDOWEVENT
                && event.window.event == SDL_WINDOWEVENT_CLOSE)
                quit = true;
            if (event.type == SDL_QUIT)
                quit = true;

            // Handle keyboard input for controller 1
            if (controller1 && (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)) {
                bool pressed = (event.type == SDL_KEYDOWN);
                switch (event.key.keysym.sym) {
                case SDLK_w: controller1->set_button(NES::Controller::BTN_UP, pressed); break;
                case SDLK_s: controller1->set_button(NES::Controller::BTN_DOWN, pressed); break;
                case SDLK_a: controller1->set_button(NES::Controller::BTN_LEFT, pressed); break;
                case SDLK_d: controller1->set_button(NES::Controller::BTN_RIGHT, pressed); break;
                case SDLK_i: controller1->set_button(NES::Controller::BTN_START, pressed); break;
                case SDLK_o: controller1->set_button(NES::Controller::BTN_SELECT, pressed); break;
                case SDLK_k: controller1->set_button(NES::Controller::BTN_A, pressed); break;
                case SDLK_l: controller1->set_button(NES::Controller::BTN_B, pressed); break;
                default: break;
                }
            }
        }

        return quit;
    }
};

}  // namespace GFX

#endif  // INC_2A03_GUI_H
