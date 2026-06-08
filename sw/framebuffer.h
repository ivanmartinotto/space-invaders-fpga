#pragma once
#include <stdint.h>

void     fb_init(void);
void     fb_cleanup(void);
void     fb_swap(void);
void     put_pixel(int x, int y, uint16_t color);
uint16_t get_pixel(int x, int y);
