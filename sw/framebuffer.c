#include "framebuffer.h"
#include "game.h"

#define FRAMEBUFFER_BASE  0x20000000u
#define FRAMEBUFFER_SIZE  (SCREEN_W * SCREEN_H * 2u)
#define CTRL_REG_BASE     0xFF200000u
#define CTRL_VSYNC_FLAG   0x2u
#define CTRL_BUF_SELECT   0x1u

static volatile uint16_t * const fb[2] = {
    (volatile uint16_t *)FRAMEBUFFER_BASE,
    (volatile uint16_t *)(FRAMEBUFFER_BASE + FRAMEBUFFER_SIZE),
};
static volatile uint32_t * const ctrl_reg = (volatile uint32_t *)CTRL_REG_BASE;
static int active_fb;

void fb_init(void) {
    active_fb = 0;
}

void fb_cleanup(void) {
    /* nada a liberar em bare-metal */
}

void fb_swap(void) {
    while (!(*ctrl_reg & CTRL_VSYNC_FLAG));
    active_fb = 1 - active_fb;
    *ctrl_reg = (*ctrl_reg & ~CTRL_BUF_SELECT) | (uint32_t)active_fb;
    *ctrl_reg &= ~CTRL_VSYNC_FLAG;
}

void put_pixel(int x, int y, uint16_t color) {
    if ((unsigned)x >= (unsigned)SCREEN_W || (unsigned)y >= (unsigned)SCREEN_H)
        return;
    fb[1 - active_fb][(unsigned)y * SCREEN_W + (unsigned)x] = color;
}

uint16_t get_pixel(int x, int y) {
    if ((unsigned)x >= (unsigned)SCREEN_W || (unsigned)y >= (unsigned)SCREEN_H)
        return COLOR_BLACK;
    return fb[1 - active_fb][(unsigned)y * SCREEN_W + (unsigned)x];
}
