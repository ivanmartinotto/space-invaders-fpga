#include "sprites.h"

/* Pixel shorthands — local to this file only */
#define W  COLOR_WHITE
#define G  COLOR_GREEN
#define R  COLOR_RED
#define Y  COLOR_YELLOW
#define M  COLOR_MAGENTA
#define _  COLOR_BLACK

/* ── Player ship (13 × 8, white) ─────────────────────────────────────────── */
static const uint16_t player_px[13 * 8] = {
    _,_,_,_,_,_,W,_,_,_,_,_,_,
    _,_,_,_,_,W,W,W,_,_,_,_,_,
    _,_,_,_,W,W,W,W,W,_,_,_,_,
    _,_,_,W,W,W,W,W,W,W,_,_,_,
    _,_,W,W,W,W,W,W,W,W,W,_,_,
    W,W,W,W,W,W,W,W,W,W,W,W,W,
    W,W,W,_,W,W,W,W,W,_,W,W,W,
    W,_,_,_,_,W,W,W,_,_,_,_,W,
};
const Sprite SPRITE_PLAYER = { 13, 8, player_px };

/* ── Invader type A (bottom 2 rows, 12 × 8, green) ──────────────────────── */
static const uint16_t inv_a0[12 * 8] = {
    _,_,G,G,_,_,_,_,G,G,_,_,
    _,_,_,G,G,_,_,G,G,_,_,_,
    _,_,G,G,G,G,G,G,G,G,_,_,
    _,G,G,_,G,G,G,G,_,G,G,_,
    G,G,G,G,G,G,G,G,G,G,G,G,
    G,_,G,G,G,G,G,G,G,G,_,G,
    G,_,_,_,G,G,G,G,_,_,_,G,
    _,_,G,G,_,_,_,_,G,G,_,_,
};
static const uint16_t inv_a1[12 * 8] = {
    _,_,G,G,_,_,_,_,G,G,_,_,
    _,_,_,G,G,_,_,G,G,_,_,_,
    _,_,G,G,G,G,G,G,G,G,_,_,
    _,G,G,_,G,G,G,G,_,G,G,_,
    G,G,G,G,G,G,G,G,G,G,G,G,
    _,G,_,G,G,G,G,G,G,_,G,_,
    G,_,_,G,G,_,_,G,G,_,_,G,
    _,G,_,_,_,_,_,_,_,_,G,_,
};
const Sprite SPRITE_INVADER_A[2] = {
    { 12, 8, inv_a0 },
    { 12, 8, inv_a1 },
};

/* ── Invader type B (middle 2 rows, 11 × 8, green) ──────────────────────── */
static const uint16_t inv_b0[11 * 8] = {
    _,G,_,_,_,_,_,_,_,G,_,
    _,_,G,_,_,_,_,_,G,_,_,
    _,G,G,G,G,G,G,G,G,G,_,
    G,G,_,G,G,G,G,G,_,G,G,
    G,G,G,G,G,G,G,G,G,G,G,
    G,_,G,G,G,G,G,G,G,_,G,
    G,_,G,_,_,_,_,_,G,_,G,
    _,_,_,G,G,_,G,G,_,_,_,
};
static const uint16_t inv_b1[11 * 8] = {
    _,G,_,_,_,_,_,_,_,G,_,
    G,_,_,G,_,_,_,G,_,_,G,
    G,G,G,G,G,G,G,G,G,G,G,
    G,_,G,G,G,G,G,G,G,_,G,
    G,G,G,G,G,G,G,G,G,G,G,
    _,G,G,G,G,G,G,G,G,G,_,
    _,_,G,_,_,_,_,_,G,_,_,
    G,_,_,_,_,_,_,_,_,_,G,
};
const Sprite SPRITE_INVADER_B[2] = {
    { 11, 8, inv_b0 },
    { 11, 8, inv_b1 },
};

/* ── Invader type C (top row, 8 × 8, green) ──────────────────────────────── */
static const uint16_t inv_c0[8 * 8] = {
    _,_,G,G,G,G,_,_,
    _,G,G,G,G,G,G,_,
    G,G,G,G,G,G,G,G,
    G,G,_,G,G,_,G,G,
    G,G,G,G,G,G,G,G,
    _,_,G,_,_,G,_,_,
    _,G,_,G,G,_,G,_,
    G,_,G,_,_,G,_,G,
};
static const uint16_t inv_c1[8 * 8] = {
    _,_,G,G,G,G,_,_,
    _,G,G,G,G,G,G,_,
    G,G,G,G,G,G,G,G,
    G,G,_,G,G,_,G,G,
    G,G,G,G,G,G,G,G,
    _,G,G,_,_,G,G,_,
    G,_,_,G,G,_,_,G,
    _,G,_,_,_,_,G,_,
};
const Sprite SPRITE_INVADER_C[2] = {
    { 8, 8, inv_c0 },
    { 8, 8, inv_c1 },
};

/* ── UFO (16 × 7, magenta) ───────────────────────────────────────────────── */
static const uint16_t ufo_px[16 * 7] = {
    _,_,_,_,M,M,M,M,M,M,M,M,_,_,_,_,
    _,_,_,M,M,M,M,M,M,M,M,M,M,_,_,_,
    _,_,M,M,M,M,M,M,M,M,M,M,M,M,_,_,
    M,M,_,M,M,_,M,M,M,M,_,M,M,_,M,M,
    M,M,M,M,M,M,M,M,M,M,M,M,M,M,M,M,
    _,_,M,M,_,_,M,M,_,_,M,M,_,_,M,_,
    _,_,_,M,_,_,_,_,_,_,_,M,_,_,_,_,
};
const Sprite SPRITE_UFO = { 16, 7, ufo_px };

/* ── Player bullet (3 × 8, white) ───────────────────────────────────────── */
static const uint16_t pbullet_px[3 * 8] = {
    _,W,_,
    _,W,_,
    _,W,_,
    W,W,W,
    _,W,_,
    _,W,_,
    _,W,_,
    W,W,W,
};
const Sprite SPRITE_BULLET_PLAYER = { 3, 8, pbullet_px };

/* ── Enemy bullet (3 × 8, yellow zigzag) ────────────────────────────────── */
static const uint16_t ebullet_px[3 * 8] = {
    _,_,Y,
    _,Y,_,
    Y,_,_,
    _,Y,_,
    _,_,Y,
    _,Y,_,
    Y,_,_,
    _,Y,_,
};
const Sprite SPRITE_BULLET_ENEMY = { 3, 8, ebullet_px };

/* ── Explosion (16 × 8, yellow/white) ───────────────────────────────────── */
static const uint16_t explosion_px[16 * 8] = {
    _,W,_,_,_,Y,_,_,_,_,W,_,_,_,Y,_,
    _,_,Y,_,W,_,_,Y,_,W,_,_,Y,_,_,_,
    _,_,_,W,_,Y,W,_,Y,_,W,Y,_,W,_,_,
    Y,_,W,Y,W,Y,W,Y,W,Y,W,Y,W,Y,_,Y,
    Y,W,Y,W,Y,W,Y,W,Y,W,Y,W,Y,W,Y,W,
    _,_,W,Y,_,Y,W,_,W,_,Y,W,_,W,_,_,
    _,_,Y,_,W,_,_,W,_,Y,_,_,W,_,_,_,
    _,W,_,_,_,W,_,_,_,_,Y,_,_,_,W,_,
};
const Sprite SPRITE_EXPLOSION = { 16, 8, explosion_px };

/* Undefine shorthands so they don't leak */
#undef W
#undef G
#undef R
#undef Y
#undef M
#undef _
