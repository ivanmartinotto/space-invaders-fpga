#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include "../input.h"
#include "../game.h"

int input_init(void) { return 0; }

void input_poll(int fd, InputState *state) {
    (void)fd;
    int fire = 0, pause = 0, quit = 0, reset = 0;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            quit = 1;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.scancode) {
            case SDL_SCANCODE_SPACE:  fire  = 1; break;
            case SDL_SCANCODE_P:      pause = 1; break;
            case SDL_SCANCODE_Q:
            case SDL_SCANCODE_ESCAPE: quit  = 1; break;
            case SDL_SCANCODE_R:      reset = 1; break;
            default: break;
            }
        }
    }

    const uint8_t *keys = SDL_GetKeyboardState(NULL);
    state->left  = keys[SDL_SCANCODE_LEFT]  || keys[SDL_SCANCODE_A];
    state->right = keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D];
    state->fire  = fire;
    state->pause = pause;
    state->quit  = quit;
    state->reset = reset;
}

void input_cleanup(int fd) { (void)fd; }
