#ifndef INC_2A03_RENDERER_H
#define INC_2A03_RENDERER_H

#include <SDL2/SDL.h>

namespace GFX {

class Renderer {
public:
    SDL_Window *w;
    SDL_Renderer *r;
    SDL_Texture *tex;

    const int display_x;
    const int display_y;

    Renderer(int display_x, int display_y) 
    : display_x(display_x), display_y(display_y) {};

    void setup_window() {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            std::cerr << "SDL_Init error: " << SDL_GetError() << std::endl;
            throw std::runtime_error("SDL_Init error");
        }

        w = SDL_CreateWindow("2A03", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                             display_x, display_y, SDL_WINDOW_SHOWN);
        if (!w) {
            std::cerr << "SDL_CreateWindow error: " << SDL_GetError() << std::endl;
            SDL_Quit();
            throw std::runtime_error("SDL_CreateWindow error");
        }

        r = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED);

        if (!r) {
            std::cerr << "SDL_CreateRenderer error: " << SDL_GetError()
                      << std::endl;
            SDL_DestroyWindow(w);
            SDL_Quit();
            throw std::runtime_error("SDL_CreateRenderer error");
        }

        tex = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888,
                                SDL_TEXTUREACCESS_STREAMING, display_x, display_y);
        if (!tex) {
            std::cerr << "SDL_CreateTexture error: " << SDL_GetError() << std::endl;
            SDL_DestroyRenderer(r);
            SDL_DestroyWindow(w);
            SDL_Quit();
            throw std::runtime_error("SDL_CreateTexture error");
        }
    }

    void draw(uint32_t *fb) {
        SDL_UpdateTexture(tex, NULL, fb, display_x * sizeof(uint32_t));
        SDL_RenderClear(r);
        SDL_RenderCopy(r, tex, NULL, NULL);
        SDL_RenderPresent(r);
    }
};

} // namespace GFX

#endif // INC_2A03_RENDERER_H
