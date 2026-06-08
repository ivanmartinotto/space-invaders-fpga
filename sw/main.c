#include "game.h"
#include "game_api.h"
#include "framebuffer.h"
#include "input.h"
#include "timer.h"
#include <stdlib.h>

#define FRAME_US  (1000000u / TARGET_FPS)

int main(void) {
    timer_init();
    srand((unsigned)timer_us());   /* semente baseada no tempo de boot */

    Game g;
    game_init(&g);

    fb_init();
    g.input_fd = input_init();

    while (g.state != STATE_QUIT) {
        uint64_t t0 = timer_us();

        input_poll(g.input_fd, &g.input);
        game_update(&g, FRAME_DT);
        game_render(&g);
        fb_swap();

        uint64_t elapsed = timer_us() - t0;
        if (elapsed < FRAME_US)
            timer_usleep((uint32_t)(FRAME_US - elapsed));
    }

    input_cleanup(g.input_fd);
    fb_cleanup();
    return 0;
}
