#pragma once
#include <stdint.h>

/* 8×8 bitmap font for ASCII 0x00–0x7F.
   Each entry is 8 bytes: one byte per row, MSB = leftmost pixel. */
extern const uint8_t font8x8_basic[128][8];
