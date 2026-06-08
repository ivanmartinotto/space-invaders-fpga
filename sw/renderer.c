#include "renderer.h"
#include "framebuffer.h"
#include "font.h"

void clear_screen(uint16_t color) {
    for (int y = 0; y < SCREEN_H; y++)
        for (int x = 0; x < SCREEN_W; x++)
            put_pixel(x, y, color);
}

void draw_rect(int x, int y, int w, int h, uint16_t color) {
    for (int row = 0; row < h; row++)
        for (int col = 0; col < w; col++)
            put_pixel(x + col, y + row, color);
}

void draw_char(int x, int y, char c, uint16_t fg, uint16_t bg) {
    unsigned idx = (unsigned)(uint8_t)c & 0x7Fu;
    for (int row = 0; row < 8; row++) {
        uint8_t bits = font8x8_basic[idx][row];
        for (int col = 0; col < 8; col++) {
            uint16_t color = (bits & (0x80u >> (unsigned)col)) ? fg : bg;
            if (color != COLOR_BLACK || bg != COLOR_BLACK)
                put_pixel(x + col, y + row, color);
        }
    }
}

void draw_string(int x, int y, const char *s, uint16_t fg, uint16_t bg) {
    int cx = x;
    for (; *s; s++, cx += 8)
        draw_char(cx, y, *s, fg, bg);
}

void draw_sprite(int x, int y, const Sprite *spr) {
    for (int row = 0; row < spr->height; row++) {
        for (int col = 0; col < spr->width; col++) {
            uint16_t px = spr->pixels[row * spr->width + col];
            if (px != COLOR_BLACK)
                put_pixel(x + col, y + row, px);
        }
    }
}
