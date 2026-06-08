#pragma once
#include <stdint.h>
#include <stddef.h>

/* ── Screen ─────────────────────────────────────────────────────────────── */
#define SCREEN_W        640
#define SCREEN_H        480
#define TARGET_FPS      60
#define FRAME_DT        (1.0f / TARGET_FPS)

/* ── Player ──────────────────────────────────────────────────────────────── */
#define PLAYER_Y            440
#define PLAYER_W            13
#define PLAYER_H            8
#define PLAYER_SPEED        2
#define PLAYER_LIVES        3
#define PLAYER_MIN_X        8
#define PLAYER_MAX_X        (SCREEN_W - PLAYER_W - 8)
#define INVINCIBLE_FRAMES   90

/* ── Bullets ─────────────────────────────────────────────────────────────── */
#define BULLET_W        3
#define BULLET_H        8
#define BULLET_P_SPEED  4
#define BULLET_E_SPEED  3
#define ENEMY_BULLETS   3

/* ── Fleet ───────────────────────────────────────────────────────────────── */
#define FLEET_ROWS              5
#define FLEET_COLS              11
#define FLEET_SPACING_X         16
#define FLEET_SPACING_Y         16
#define FLEET_START_X           80
#define FLEET_START_Y           64
#define FLEET_STEP_INIT_PX      2
#define FLEET_STEP_INIT_INT     0.5f
#define FLEET_STEP_MIN_INT      0.05f
#define FLEET_STEP_INT_DELTA    0.008f
#define FLEET_DROP_PX           8
#define FLEET_SHOOT_INIT_INT    1.5f
#define FLEET_SHOOT_MIN_INT     0.3f
#define FLEET_BOUNDARY_LEFT     8
#define FLEET_BOUNDARY_RIGHT    (SCREEN_W - 8)

/* ── UFO ─────────────────────────────────────────────────────────────────── */
#define UFO_Y           40
#define UFO_W           16
#define UFO_H           7
#define UFO_SPEED       1
#define UFO_SPAWN_MIN   25.0f
#define UFO_SPAWN_MAX   30.0f

/* ── Bunkers ─────────────────────────────────────────────────────────────── */
#define BUNKER_COUNT    4
#define BUNKER_COLS     12
#define BUNKER_ROWS     8
#define BUNKER_CELL     3
#define BUNKER_W        (BUNKER_COLS * BUNKER_CELL)
#define BUNKER_H        (BUNKER_ROWS * BUNKER_CELL)
#define BUNKER_Y        380

/* ── Colors (RGB565) ─────────────────────────────────────────────────────── */
#define RGB565(r,g,b) ((uint16_t)((((uint16_t)(r)>>3u)<<11u) \
                                 |(((uint16_t)(g)>>2u)<<5u)  \
                                 | ((uint16_t)(b)>>3u)))
#define COLOR_BLACK     ((uint16_t)0x0000u)
#define COLOR_WHITE     ((uint16_t)0xFFFFu)
#define COLOR_GREEN     ((uint16_t)0x07E0u)
#define COLOR_RED       ((uint16_t)0xF800u)
#define COLOR_YELLOW    ((uint16_t)0xFFE0u)
#define COLOR_CYAN      ((uint16_t)0x07FFu)
#define COLOR_MAGENTA   ((uint16_t)0xF81Fu)
#define COLOR_DARK_GREEN ((uint16_t)0x0380u)

/* ── Types ───────────────────────────────────────────────────────────────── */
typedef struct { int x, y; } Vec2;

typedef struct {
    int             width, height;
    const uint16_t *pixels;
} Sprite;

typedef struct {
    Vec2 pos;
    int  active;
    int  dy, dx;
} Bullet;

typedef struct {
    Vec2   pos;
    int    lives;
    Bullet bullet;
    int    invincible_frames;
} Player;

typedef struct {
    Vec2 pos;
    int  alive;
    int  type;        /* 0=A bottom rows, 1=B middle rows, 2=C top row */
    int  anim_frame;
} Invader;

typedef struct {
    Invader grid[FLEET_ROWS][FLEET_COLS];
    Vec2    origin;
    int     dir;
    int     step_px;
    int     alive_count;
    float   step_interval;
    float   step_timer;
    Bullet  bullets[ENEMY_BULLETS];
    float   shoot_timer;
    float   shoot_interval;
    int     anim_frame;
} Fleet;

typedef struct {
    Vec2  pos;
    int   active;
    int   dx;
    int   points;
    float spawn_timer;
} UFO;

typedef struct {
    Vec2    origin;
    uint8_t cells[BUNKER_ROWS][BUNKER_COLS];
} Bunker;

typedef enum {
    STATE_TITLE,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_NEXT_LEVEL,
    STATE_GAME_OVER,
    STATE_QUIT
} GameState_e;

typedef struct {
    int left, right, fire, pause, quit, reset;
} InputState;

typedef struct {
    Player      player;
    Fleet       fleet;
    UFO         ufo;
    Bunker      bunkers[BUNKER_COUNT];
    int         score;
    int         high_score;
    int         level;
    GameState_e state;
    float       state_timer;
    InputState  input;
    int         input_fd;
    int         shot_count;
} Game;
