#pragma once
#include "game.h"

void draw_rect(int x, int y, int w, int h, uint16_t color);
void draw_char(int x, int y, char c, uint16_t fg, uint16_t bg);
void draw_string(int x, int y, const char *s, uint16_t fg, uint16_t bg);
void draw_sprite(int x, int y, const Sprite *spr);
void clear_screen(uint16_t color);
