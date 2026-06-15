#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include "../framebuffer.h"
#include "../game.h"

static SDL_Window   *s_window;
static SDL_Renderer *s_renderer;
static SDL_Texture  *s_texture;
static uint16_t      s_pixels[SCREEN_H * SCREEN_W];

void fb_init(void) {
    SDL_SetMainReady();
    SDL_Init(SDL_INIT_VIDEO);
    s_window = SDL_CreateWindow("Space Invaders",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_W, SCREEN_H, 0);
    s_renderer = SDL_CreateRenderer(s_window, -1, SDL_RENDERER_PRESENTVSYNC);
    s_texture  = SDL_CreateTexture(s_renderer,
        SDL_PIXELFORMAT_RGB565,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_W, SCREEN_H);
}

void fb_cleanup(void) {
    SDL_DestroyTexture(s_texture);
    SDL_DestroyRenderer(s_renderer);
    SDL_DestroyWindow(s_window);
    SDL_Quit();
}

void fb_swap(void) {
    SDL_UpdateTexture(s_texture, NULL, s_pixels, SCREEN_W * sizeof(uint16_t));
    SDL_RenderCopy(s_renderer, s_texture, NULL, NULL);
    SDL_RenderPresent(s_renderer);
}

void put_pixel(int x, int y, uint16_t color) {
    if ((unsigned)x >= (unsigned)SCREEN_W || (unsigned)y >= (unsigned)SCREEN_H)
        return;
    s_pixels[(unsigned)y * SCREEN_W + (unsigned)x] = color;
}

uint16_t get_pixel(int x, int y) {
    if ((unsigned)x >= (unsigned)SCREEN_W || (unsigned)y >= (unsigned)SCREEN_H)
        return 0;
    return s_pixels[(unsigned)y * SCREEN_W + (unsigned)x];
}
