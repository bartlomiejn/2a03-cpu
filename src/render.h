#ifndef INC_2A03_RENDERER_H
#define INC_2A03_RENDERER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

namespace GFX {

class Renderer {
   public:
    SDL_Window *wnd_main;
    SDL_Renderer *ren_main;
    SDL_Texture *tex_main;

    SDL_Window *wnd_chr;
    SDL_Renderer *ren_chr;
    SDL_Surface *surf_chr;
    SDL_Texture *tex_chr;

    TTF_Font *gui_font;

    const SDL_Color gui_text_color = { 0, 0, 0 };
    const std::string gui_font_name = "Inter-VariableFont.ttf";

    const SDL_Rect chr_inputbox_rect = { 0, 0, 200, 50 }; 

    const int display_x;
    const int display_y;

    Renderer(int display_x, int display_y)
        : wnd_main(nullptr),
          ren_main(nullptr),
          tex_main(nullptr),
          wnd_chr(nullptr),
          ren_chr(nullptr),
          tex_chr(nullptr),
          gui_font(nullptr),
          display_x(display_x),
          display_y(display_y) {};

    void setup() {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            std::cerr << "SDL_Init error: " << SDL_GetError() << std::endl;
            throw std::runtime_error("SDL_Init error");
        }
        if (TTF_Init() == -1) {
            std::cerr << "TTF_Init error: " << TTF_GetError() << std::endl;
            throw std::runtime_error("TTF_Init error");
        }

        gui_font = TTF_OpenFont(gui_font_name.c_str(), 24);
        if (!gui_font) {
            std::cerr << "TTF_OpenFont failed:" << TTF_GetError();
            throw std::runtime_error("TTF_OpenFont error");
        }

        wnd_main = SDL_CreateWindow("2A03", SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED, display_x, display_y,
                             SDL_WINDOW_SHOWN);
        if (!wnd_main) {
            std::cerr << "SDL_CreateWindow error: " << SDL_GetError()
                      << std::endl;
            SDL_Quit();
            throw std::runtime_error("SDL_CreateWindow error");
        }

        ren_main = SDL_CreateRenderer(wnd_main, -1, SDL_RENDERER_ACCELERATED);
        if (!ren_main) {
            std::cerr << "SDL_CreateRenderer error: " << SDL_GetError()
                      << std::endl;
            SDL_DestroyWindow(wnd_main);
            SDL_Quit();
            throw std::runtime_error("SDL_CreateRenderer error");
        }

        tex_main = SDL_CreateTexture(ren_main, SDL_PIXELFORMAT_RGBA8888,
                                SDL_TEXTUREACCESS_STREAMING, display_x,
                                display_y);
        if (!tex_main) {
            std::cerr << "SDL_CreateTexture error: " << SDL_GetError()
                      << std::endl;
            SDL_DestroyRenderer(ren_main);
            SDL_DestroyWindow(wnd_main);
            SDL_Quit();
            throw std::runtime_error("SDL_CreateTexture error");
        }

        setup_chr_window();
    }

    void setup_chr_window() {
        wnd_chr = SDL_CreateWindow("CHR Viewer", SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED, display_x, display_y,
                             SDL_WINDOW_SHOWN);
        if (!wnd_chr) {
            std::cerr << "SDL_CreateWindow error: " << SDL_GetError()
                      << std::endl;
            SDL_Quit();
            throw std::runtime_error("SDL_CreateWindow error");
        }

        ren_chr = SDL_CreateRenderer(wnd_chr, -1, SDL_RENDERER_ACCELERATED);
        if (!ren_chr) {
            std::cerr << "SDL_CreateRenderer error: " << SDL_GetError()
                      << std::endl;
            SDL_DestroyWindow(wnd_chr);
            SDL_Quit();
            throw std::runtime_error("SDL_CreateRenderer error");
        }

        tex_chr = SDL_CreateTexture(ren_chr, SDL_PIXELFORMAT_RGBA8888,
                                SDL_TEXTUREACCESS_STREAMING, display_x,
                                display_y);
        if (!tex_chr) {
            std::cerr << "SDL_CreateTexture error: " << SDL_GetError()
                      << std::endl;
            SDL_DestroyRenderer(ren_chr);
            SDL_DestroyWindow(wnd_chr);
            SDL_Quit();
            throw std::runtime_error("SDL_CreateTexture error");
        }
    }

    void draw(uint32_t *fb) {
        SDL_UpdateTexture(tex_main, NULL, fb, display_x * sizeof(uint32_t));
        SDL_RenderClear(ren_main);
        SDL_RenderCopy(ren_main, tex_main, NULL, NULL);
        SDL_RenderPresent(ren_main);
    }

    void draw_chr(NES::iNESv1::Mapper::Base &mapper) {
        
    }

    static bool poll_quit() {
        SDL_Event event;
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            return true;
        } else {
            return false;
        }
    }
};

}  // namespace GFX

#endif  // INC_2A03_RENDERER_H
