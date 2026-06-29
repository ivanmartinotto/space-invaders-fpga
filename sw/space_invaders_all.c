/* ============================================================================
 * space_invaders_all.c — AMALGAMA: todo o jogo num unico arquivo C.
 * Gerado a partir dos modulos sw (.c e .h) (backend bare-metal Zynq/Vitis).
 * Build alvo: Vitis standalone (Zybo Z7-20). Exclui sw/host/ (SDL).
 * Para regenerar: rode sw/make_amalgam.sh.
 * ========================================================================== */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "xil_io.h"
#include "xil_cache.h"
#include "xtime_l.h"
#include "sleep.h"

/* ===== game.h ===== */

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

/* ===== game_api.h ===== */

void game_init(Game *g);
void game_update(Game *g, float dt);
void game_render(const Game *g);

/* ===== timer.h ===== */

/* Build Vitis (alvo): timer.c usa XTime_GetTime + COUNTS_PER_SECOND do BSP,
   entao estas constantes nao sao usadas. Mantidas para a build host (SDL) e
   para a build legacy raw-gcc (legacy-gcc/), que nao tem o BSP do Vitis.
   Zybo Z7-20: ARM Cortex-A9 @ 667 MHz -> PERIPHCLK = 333.5 MHz. */

#define CPU_FREQ_HZ     667000000UL
#define PERIPHCLK_HZ    (CPU_FREQ_HZ / 2UL)   /* ~333 MHz */

void     timer_init(void);
uint64_t timer_us(void);
void     timer_usleep(uint32_t us);

/* ===== input.h ===== */

int  input_init(void);
void input_poll(int fd, InputState *state);
void input_cleanup(int fd);

/* ===== font.h ===== */

/* 8×8 bitmap font for ASCII 0x00–0x7F.
   Each entry is 8 bytes: one byte per row, MSB = leftmost pixel. */
extern const uint8_t font8x8_basic[128][8];

/* ===== player.h ===== */

void player_init(Player *p);
void player_update(Player *p, const InputState *input, int *shot_count);

/* ===== bullet.h ===== */

void bullet_update(Bullet *b);
void bullet_fire(Bullet *b, int x, int y, int dx, int dy);
void bullet_clear(Bullet *b);

/* ===== bunker.h ===== */

void bunker_init(Bunker *b, int origin_x, int origin_y);
void bunkers_init(Bunker bunkers[BUNKER_COUNT]);
int  bunker_damage_bullet(Bunker *b, Bullet *bullet);

/* ===== invaders.h ===== */

void fleet_init(Fleet *f, int level);
void fleet_update(Fleet *f, float dt);
int  fleet_alive_count(const Fleet *f);

/* ===== ufo.h ===== */

void ufo_init(UFO *u);
void ufo_update(UFO *u, float dt);

/* ===== collision.h ===== */

/* Returns 1 if the two axis-aligned boxes overlap */
int aabb_overlap(int ax, int ay, int aw, int ah,
                 int bx, int by, int bw, int bh);

/* Process all collisions for one frame. Returns:
     0 = nothing special
     1 = player killed (lives--; if lives==0 caller sets GAME_OVER)
     2 = all invaders cleared (caller advances level)  */
int collisions_update(Game *g);

/* ===== renderer.h ===== */

void draw_rect(int x, int y, int w, int h, uint16_t color);
void draw_char(int x, int y, char c, uint16_t fg, uint16_t bg);
void draw_string(int x, int y, const char *s, uint16_t fg, uint16_t bg);
void draw_sprite(int x, int y, const Sprite *spr);
void clear_screen(uint16_t color);

/* ===== framebuffer.h ===== */

void     fb_init(void);
void     fb_cleanup(void);
void     fb_swap(void);
void     put_pixel(int x, int y, uint16_t color);
uint16_t get_pixel(int x, int y);

/* ===== assets/sprites.h ===== */

extern const Sprite SPRITE_PLAYER;
extern const Sprite SPRITE_INVADER_A[2];
extern const Sprite SPRITE_INVADER_B[2];
extern const Sprite SPRITE_INVADER_C[2];
extern const Sprite SPRITE_UFO;
extern const Sprite SPRITE_BULLET_PLAYER;
extern const Sprite SPRITE_BULLET_ENEMY;
extern const Sprite SPRITE_EXPLOSION;

/* ===== assets/font8x8.c ===== */

/* Standard 8×8 bitmap font (CP437/IBM PC BIOS, public domain).
   Each character: 8 bytes, each byte = 1 row, bit 0 = leftmost pixel. */
const uint8_t font8x8_basic[128][8] = {
    /* 0x00 */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x01 */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x02 */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x03 */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x04 */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x05 */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x06 */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x07 */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x08 */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x09 */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x0A */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x0B */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x0C */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x0D */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x0E */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x0F */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x10 */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x11 */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x12 */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x13 */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x14 */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x15 */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x16 */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x17 */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x18 */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x19 */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x1A */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x1B */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x1C */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x1D */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x1E */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x1F */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x20   */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, /* space */
    /* 0x21 ! */ { 0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00 },
    /* 0x22 " */ { 0x36,0x36,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x23 # */ { 0x36,0x36,0x7F,0x36,0x7F,0x36,0x36,0x00 },
    /* 0x24 $ */ { 0x0C,0x3E,0x03,0x1E,0x30,0x1F,0x0C,0x00 },
    /* 0x25 % */ { 0x00,0x63,0x33,0x18,0x0C,0x66,0x63,0x00 },
    /* 0x26 & */ { 0x1C,0x36,0x1C,0x6E,0x3B,0x33,0x6E,0x00 },
    /* 0x27 ' */ { 0x06,0x06,0x03,0x00,0x00,0x00,0x00,0x00 },
    /* 0x28 ( */ { 0x18,0x0C,0x06,0x06,0x06,0x0C,0x18,0x00 },
    /* 0x29 ) */ { 0x06,0x0C,0x18,0x18,0x18,0x0C,0x06,0x00 },
    /* 0x2A * */ { 0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00 },
    /* 0x2B + */ { 0x00,0x0C,0x0C,0x3F,0x0C,0x0C,0x00,0x00 },
    /* 0x2C , */ { 0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x06 },
    /* 0x2D - */ { 0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x00 },
    /* 0x2E . */ { 0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x00 },
    /* 0x2F / */ { 0x60,0x30,0x18,0x0C,0x06,0x03,0x01,0x00 },
    /* 0x30 0 */ { 0x3E,0x63,0x73,0x7B,0x6F,0x67,0x3E,0x00 },
    /* 0x31 1 */ { 0x0C,0x0E,0x0C,0x0C,0x0C,0x0C,0x3F,0x00 },
    /* 0x32 2 */ { 0x1E,0x33,0x30,0x1C,0x06,0x33,0x3F,0x00 },
    /* 0x33 3 */ { 0x1E,0x33,0x30,0x1C,0x30,0x33,0x1E,0x00 },
    /* 0x34 4 */ { 0x38,0x3C,0x36,0x33,0x7F,0x30,0x78,0x00 },
    /* 0x35 5 */ { 0x3F,0x03,0x1F,0x30,0x30,0x33,0x1E,0x00 },
    /* 0x36 6 */ { 0x1C,0x06,0x03,0x1F,0x33,0x33,0x1E,0x00 },
    /* 0x37 7 */ { 0x3F,0x33,0x30,0x18,0x0C,0x0C,0x0C,0x00 },
    /* 0x38 8 */ { 0x1E,0x33,0x33,0x1E,0x33,0x33,0x1E,0x00 },
    /* 0x39 9 */ { 0x1E,0x33,0x33,0x3E,0x30,0x18,0x0E,0x00 },
    /* 0x3A : */ { 0x00,0x0C,0x0C,0x00,0x00,0x0C,0x0C,0x00 },
    /* 0x3B ; */ { 0x00,0x0C,0x0C,0x00,0x00,0x0C,0x0C,0x06 },
    /* 0x3C < */ { 0x18,0x0C,0x06,0x03,0x06,0x0C,0x18,0x00 },
    /* 0x3D = */ { 0x00,0x00,0x3F,0x00,0x00,0x3F,0x00,0x00 },
    /* 0x3E > */ { 0x06,0x0C,0x18,0x30,0x18,0x0C,0x06,0x00 },
    /* 0x3F ? */ { 0x1E,0x33,0x30,0x18,0x0C,0x00,0x0C,0x00 },
    /* 0x40 @ */ { 0x3E,0x63,0x7B,0x7B,0x7B,0x03,0x1E,0x00 },
    /* 0x41 A */ { 0x0C,0x1E,0x33,0x33,0x3F,0x33,0x33,0x00 },
    /* 0x42 B */ { 0x3F,0x66,0x66,0x3E,0x66,0x66,0x3F,0x00 },
    /* 0x43 C */ { 0x3C,0x66,0x03,0x03,0x03,0x66,0x3C,0x00 },
    /* 0x44 D */ { 0x1F,0x36,0x66,0x66,0x66,0x36,0x1F,0x00 },
    /* 0x45 E */ { 0x7F,0x46,0x16,0x1E,0x16,0x46,0x7F,0x00 },
    /* 0x46 F */ { 0x7F,0x46,0x16,0x1E,0x16,0x06,0x0F,0x00 },
    /* 0x47 G */ { 0x3C,0x66,0x03,0x03,0x73,0x66,0x7C,0x00 },
    /* 0x48 H */ { 0x33,0x33,0x33,0x3F,0x33,0x33,0x33,0x00 },
    /* 0x49 I */ { 0x1E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00 },
    /* 0x4A J */ { 0x78,0x30,0x30,0x30,0x33,0x33,0x1E,0x00 },
    /* 0x4B K */ { 0x67,0x66,0x36,0x1E,0x36,0x66,0x67,0x00 },
    /* 0x4C L */ { 0x0F,0x06,0x06,0x06,0x46,0x66,0x7F,0x00 },
    /* 0x4D M */ { 0x63,0x77,0x7F,0x7F,0x6B,0x63,0x63,0x00 },
    /* 0x4E N */ { 0x63,0x67,0x6F,0x7B,0x73,0x63,0x63,0x00 },
    /* 0x4F O */ { 0x1C,0x36,0x63,0x63,0x63,0x36,0x1C,0x00 },
    /* 0x50 P */ { 0x3F,0x66,0x66,0x3E,0x06,0x06,0x0F,0x00 },
    /* 0x51 Q */ { 0x1E,0x33,0x33,0x33,0x3B,0x1E,0x38,0x00 },
    /* 0x52 R */ { 0x3F,0x66,0x66,0x3E,0x36,0x66,0x67,0x00 },
    /* 0x53 S */ { 0x1E,0x33,0x07,0x0E,0x38,0x33,0x1E,0x00 },
    /* 0x54 T */ { 0x3F,0x2D,0x0C,0x0C,0x0C,0x0C,0x1E,0x00 },
    /* 0x55 U */ { 0x33,0x33,0x33,0x33,0x33,0x33,0x3F,0x00 },
    /* 0x56 V */ { 0x33,0x33,0x33,0x33,0x33,0x1E,0x0C,0x00 },
    /* 0x57 W */ { 0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00 },
    /* 0x58 X */ { 0x63,0x63,0x36,0x1C,0x1C,0x36,0x63,0x00 },
    /* 0x59 Y */ { 0x33,0x33,0x33,0x1E,0x0C,0x0C,0x1E,0x00 },
    /* 0x5A Z */ { 0x7F,0x63,0x31,0x18,0x4C,0x66,0x7F,0x00 },
    /* 0x5B [ */ { 0x1E,0x06,0x06,0x06,0x06,0x06,0x1E,0x00 },
    /* 0x5C \ */ { 0x03,0x06,0x0C,0x18,0x30,0x60,0x40,0x00 },
    /* 0x5D ] */ { 0x1E,0x18,0x18,0x18,0x18,0x18,0x1E,0x00 },
    /* 0x5E ^ */ { 0x08,0x1C,0x36,0x63,0x00,0x00,0x00,0x00 },
    /* 0x5F _ */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF },
    /* 0x60 ` */ { 0x0C,0x0C,0x18,0x00,0x00,0x00,0x00,0x00 },
    /* 0x61 a */ { 0x00,0x00,0x1E,0x30,0x3E,0x33,0x6E,0x00 },
    /* 0x62 b */ { 0x07,0x06,0x06,0x3E,0x66,0x66,0x3B,0x00 },
    /* 0x63 c */ { 0x00,0x00,0x1E,0x33,0x03,0x33,0x1E,0x00 },
    /* 0x64 d */ { 0x38,0x30,0x30,0x3E,0x33,0x33,0x6E,0x00 },
    /* 0x65 e */ { 0x00,0x00,0x1E,0x33,0x3F,0x03,0x1E,0x00 },
    /* 0x66 f */ { 0x1C,0x36,0x06,0x0F,0x06,0x06,0x0F,0x00 },
    /* 0x67 g */ { 0x00,0x00,0x6E,0x33,0x33,0x3E,0x30,0x1F },
    /* 0x68 h */ { 0x07,0x06,0x36,0x6E,0x66,0x66,0x67,0x00 },
    /* 0x69 i */ { 0x0C,0x00,0x0E,0x0C,0x0C,0x0C,0x1E,0x00 },
    /* 0x6A j */ { 0x30,0x00,0x30,0x30,0x30,0x33,0x33,0x1E },
    /* 0x6B k */ { 0x07,0x06,0x66,0x36,0x1E,0x36,0x67,0x00 },
    /* 0x6C l */ { 0x0E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00 },
    /* 0x6D m */ { 0x00,0x00,0x33,0x7F,0x7F,0x6B,0x63,0x00 },
    /* 0x6E n */ { 0x00,0x00,0x1F,0x33,0x33,0x33,0x33,0x00 },
    /* 0x6F o */ { 0x00,0x00,0x1E,0x33,0x33,0x33,0x1E,0x00 },
    /* 0x70 p */ { 0x00,0x00,0x3B,0x66,0x66,0x3E,0x06,0x0F },
    /* 0x71 q */ { 0x00,0x00,0x6E,0x33,0x33,0x3E,0x30,0x78 },
    /* 0x72 r */ { 0x00,0x00,0x3B,0x6E,0x66,0x06,0x0F,0x00 },
    /* 0x73 s */ { 0x00,0x00,0x3E,0x03,0x1E,0x30,0x1F,0x00 },
    /* 0x74 t */ { 0x08,0x0C,0x3E,0x0C,0x0C,0x2C,0x18,0x00 },
    /* 0x75 u */ { 0x00,0x00,0x33,0x33,0x33,0x33,0x6E,0x00 },
    /* 0x76 v */ { 0x00,0x00,0x33,0x33,0x33,0x1E,0x0C,0x00 },
    /* 0x77 w */ { 0x00,0x00,0x63,0x6B,0x7F,0x7F,0x36,0x00 },
    /* 0x78 x */ { 0x00,0x00,0x63,0x36,0x1C,0x36,0x63,0x00 },
    /* 0x79 y */ { 0x00,0x00,0x33,0x33,0x33,0x3E,0x30,0x1F },
    /* 0x7A z */ { 0x00,0x00,0x3F,0x19,0x0C,0x26,0x3F,0x00 },
    /* 0x7B { */ { 0x38,0x0C,0x0C,0x07,0x0C,0x0C,0x38,0x00 },
    /* 0x7C | */ { 0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00 },
    /* 0x7D } */ { 0x07,0x0C,0x0C,0x38,0x0C,0x0C,0x07,0x00 },
    /* 0x7E ~ */ { 0x6E,0x3B,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* 0x7F   */ { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
};

/* ===== assets/sprites.c ===== */

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

/* ===== bullet.c ===== */

void bullet_fire(Bullet *b, int x, int y, int dx, int dy) {
    b->pos.x = x;
    b->pos.y = y;
    b->dx    = dx;
    b->dy    = dy;
    b->active = 1;
}

void bullet_clear(Bullet *b) {
    b->active = 0;
}

void bullet_update(Bullet *b) {
    if (!b->active) return;
    b->pos.x += b->dx;
    b->pos.y += b->dy;
    if (b->pos.y < 0 || b->pos.y >= SCREEN_H)
        b->active = 0;
}

/* ===== player.c ===== */

void player_init(Player *p) {
    p->pos.x            = (SCREEN_W - PLAYER_W) / 2;
    p->pos.y            = PLAYER_Y;
    p->lives            = PLAYER_LIVES;
    p->invincible_frames = 0;
    bullet_clear(&p->bullet);
}

void player_update(Player *p, const InputState *input, int *shot_count) {
    /* Movement */
    if (input->left) {
        p->pos.x -= PLAYER_SPEED;
        if (p->pos.x < PLAYER_MIN_X) p->pos.x = PLAYER_MIN_X;
    }
    if (input->right) {
        p->pos.x += PLAYER_SPEED;
        if (p->pos.x > PLAYER_MAX_X) p->pos.x = PLAYER_MAX_X;
    }

    /* Fire — one bullet at a time */
    if (input->fire && !p->bullet.active) {
        int bx = p->pos.x + PLAYER_W / 2 - BULLET_W / 2;
        int by = p->pos.y - BULLET_H;
        bullet_fire(&p->bullet, bx, by, 0, -BULLET_P_SPEED);
        (*shot_count)++;
    }

    bullet_update(&p->bullet);

    if (p->invincible_frames > 0)
        p->invincible_frames--;
}

/* ===== ufo.c ===== */

static const int UFO_POINTS[] = { 50, 100, 150, 300 };

void ufo_init(UFO *u) {
    u->active      = 0;
    u->spawn_timer = UFO_SPAWN_MIN +
                     (float)rand() / (float)RAND_MAX * (UFO_SPAWN_MAX - UFO_SPAWN_MIN);
}

void ufo_update(UFO *u, float dt) {
    if (!u->active) {
        u->spawn_timer -= dt;
        if (u->spawn_timer <= 0.0f) {
            /* Spawn from left or right randomly */
            if (rand() & 1) {
                u->pos.x = -UFO_W;
                u->dx    = UFO_SPEED;
            } else {
                u->pos.x = SCREEN_W;
                u->dx    = -UFO_SPEED;
            }
            u->pos.y  = UFO_Y;
            u->active = 1;
            u->points = UFO_POINTS[rand() % 4];
        }
        return;
    }

    u->pos.x += u->dx;
    if (u->pos.x < -UFO_W || u->pos.x > SCREEN_W) {
        u->active      = 0;
        u->spawn_timer = UFO_SPAWN_MIN +
                         (float)rand() / (float)RAND_MAX * (UFO_SPAWN_MAX - UFO_SPAWN_MIN);
    }
}

/* ===== bunker.c ===== */

/* X positions for 4 evenly spaced bunkers */
static const int BUNKER_X[BUNKER_COUNT] = { 72, 200, 328, 456 };

/* Classic arch shape: 1=solid, 0=empty (notched bottom-middle and corners) */
static const uint8_t BUNKER_TEMPLATE[BUNKER_ROWS][BUNKER_COLS] = {
    { 0,0,1,1,1,1,1,1,1,1,0,0 },
    { 0,1,1,1,1,1,1,1,1,1,1,0 },
    { 1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,0,0,0,0,0,0,1,1,1 },
    { 1,1,0,0,0,0,0,0,0,0,1,1 },
};

void bunker_init(Bunker *b, int origin_x, int origin_y) {
    b->origin.x = origin_x;
    b->origin.y = origin_y;
    for (int r = 0; r < BUNKER_ROWS; r++)
        for (int c = 0; c < BUNKER_COLS; c++)
            b->cells[r][c] = BUNKER_TEMPLATE[r][c];
}

void bunkers_init(Bunker bunkers[BUNKER_COUNT]) {
    for (int i = 0; i < BUNKER_COUNT; i++)
        bunker_init(&bunkers[i], BUNKER_X[i], BUNKER_Y);
}

/* Returns 1 if bullet hit bunker and was consumed. */
int bunker_damage_bullet(Bunker *b, Bullet *bullet) {
    if (!bullet->active) return 0;

    int bx = bullet->pos.x;
    int by = bullet->pos.y;

    /* Check each pixel of bullet's 3×8 bounding box against bunker cells */
    for (int py = by; py < by + BULLET_H; py++) {
        for (int px = bx; px < bx + BULLET_W; px++) {
            int cx = (px - b->origin.x) / BUNKER_CELL;
            int cy = (py - b->origin.y) / BUNKER_CELL;
            if (cx < 0 || cx >= BUNKER_COLS || cy < 0 || cy >= BUNKER_ROWS)
                continue;
            if (b->cells[cy][cx]) {
                b->cells[cy][cx] = 0;
                /* Also destroy adjacent cells for blast radius */
                if (cx > 0)             b->cells[cy][cx-1] = 0;
                if (cx < BUNKER_COLS-1) b->cells[cy][cx+1] = 0;
                bullet->active = 0;
                return 1;
            }
        }
    }
    return 0;
}

/* ===== invaders.c ===== */

/* Row → invader type mapping:
   row 0 = type C (top), rows 1-2 = type B, rows 3-4 = type A (bottom) */
static int row_to_type(int row) {
    if (row == 0)          return 2;
    if (row <= 2)          return 1;
    return 0;
}

/* Width of invader sprite by type */
static int inv_width(int type) {
    if (type == 0) return 12;
    if (type == 1) return 11;
    return 8;
}

void fleet_init(Fleet *f, int level) {
    f->origin.x     = FLEET_START_X;
    f->origin.y     = FLEET_START_Y;
    f->dir          = 1;
    f->step_px      = FLEET_STEP_INIT_PX;
    f->anim_frame   = 0;

    /* Speed scales with level */
    float speed_bonus = (level - 1) * 0.04f;
    f->step_interval  = FLEET_STEP_INIT_INT - speed_bonus;
    if (f->step_interval < FLEET_STEP_MIN_INT)
        f->step_interval = FLEET_STEP_MIN_INT;
    f->step_timer     = f->step_interval;

    f->shoot_interval = FLEET_SHOOT_INIT_INT;
    f->shoot_timer    = f->shoot_interval;
    f->alive_count    = 0;

    for (int r = 0; r < FLEET_ROWS; r++) {
        for (int c = 0; c < FLEET_COLS; c++) {
            Invader *inv = &f->grid[r][c];
            int type     = row_to_type(r);
            inv->type       = type;
            inv->alive      = 1;
            inv->anim_frame = 0;
            inv->pos.x      = f->origin.x + c * FLEET_SPACING_X;
            inv->pos.y      = f->origin.y + r * FLEET_SPACING_Y;
            f->alive_count++;
        }
    }

    for (int i = 0; i < ENEMY_BULLETS; i++)
        bullet_clear(&f->bullets[i]);
}

/* Find rightmost and leftmost X extremes of alive invaders */
static void fleet_bounds(const Fleet *f, int *left_x, int *right_x) {
    *left_x  = SCREEN_W;
    *right_x = 0;
    for (int r = 0; r < FLEET_ROWS; r++) {
        for (int c = 0; c < FLEET_COLS; c++) {
            const Invader *inv = &f->grid[r][c];
            if (!inv->alive) continue;
            int w = inv_width(inv->type);
            if (inv->pos.x < *left_x)          *left_x  = inv->pos.x;
            if (inv->pos.x + w > *right_x)      *right_x = inv->pos.x + w;
        }
    }
}

/* Move all alive invaders by (dx, dy) */
static void fleet_translate(Fleet *f, int dx, int dy) {
    for (int r = 0; r < FLEET_ROWS; r++) {
        for (int c = 0; c < FLEET_COLS; c++) {
            if (!f->grid[r][c].alive) continue;
            f->grid[r][c].pos.x += dx;
            f->grid[r][c].pos.y += dy;
        }
    }
}

/* Pick a random alive invader from the bottom of a random column to shoot. */
static void fleet_try_shoot(Fleet *f) {
    /* Find an inactive bullet slot */
    Bullet *slot = NULL;
    for (int i = 0; i < ENEMY_BULLETS; i++) {
        if (!f->bullets[i].active) { slot = &f->bullets[i]; break; }
    }
    if (!slot) return;

    /* Pick random column, find lowest alive invader in it */
    int start_col = rand() % FLEET_COLS;
    for (int dc = 0; dc < FLEET_COLS; dc++) {
        int col = (start_col + dc) % FLEET_COLS;
        for (int r = FLEET_ROWS - 1; r >= 0; r--) {
            Invader *inv = &f->grid[r][col];
            if (!inv->alive) continue;
            int w = inv_width(inv->type);
            int bx = inv->pos.x + w / 2 - BULLET_W / 2;
            int by = inv->pos.y + 8;
            bullet_fire(slot, bx, by, 0, BULLET_E_SPEED);
            return;
        }
    }
}

void fleet_update(Fleet *f, float dt) {
    /* Update enemy bullets */
    for (int i = 0; i < ENEMY_BULLETS; i++)
        bullet_update(&f->bullets[i]);

    /* Step timer */
    f->step_timer -= dt;
    if (f->step_timer > 0.0f) {
        /* Shooting on its own timer */
        f->shoot_timer -= dt;
        if (f->shoot_timer <= 0.0f) {
            fleet_try_shoot(f);
            f->shoot_timer = f->shoot_interval;
        }
        return;
    }
    f->step_timer = f->step_interval;

    /* Take one step */
    int left_x, right_x;
    fleet_bounds(f, &left_x, &right_x);

    int next_left  = left_x  + f->dir * f->step_px;
    int next_right = right_x + f->dir * f->step_px;

    if (next_left < FLEET_BOUNDARY_LEFT || next_right > FLEET_BOUNDARY_RIGHT) {
        /* Hit wall: drop and reverse */
        fleet_translate(f, 0, FLEET_DROP_PX);
        f->dir = -f->dir;
    } else {
        fleet_translate(f, f->dir * f->step_px, 0);
    }

    /* Toggle animation frame */
    f->anim_frame = 1 - f->anim_frame;
    for (int r = 0; r < FLEET_ROWS; r++)
        for (int c = 0; c < FLEET_COLS; c++)
            if (f->grid[r][c].alive)
                f->grid[r][c].anim_frame = f->anim_frame;

    /* Adjust shoot interval with alive count */
    float t = (float)f->alive_count / (float)(FLEET_ROWS * FLEET_COLS);
    f->shoot_interval = FLEET_SHOOT_INIT_INT * t + FLEET_SHOOT_MIN_INT;
    f->shoot_timer -= dt;
    if (f->shoot_timer <= 0.0f) {
        fleet_try_shoot(f);
        f->shoot_timer = f->shoot_interval;
    }
}

int fleet_alive_count(const Fleet *f) {
    return f->alive_count;
}

/* ===== collision.c ===== */

int aabb_overlap(int ax, int ay, int aw, int ah,
                 int bx, int by, int bw, int bh) {
    return ax < bx + bw && ax + aw > bx &&
           ay < by + bh && ay + ah > by;
}

/* Invader widths by type */
static int inv_w(int type) {
    if (type == 0) return 12;
    if (type == 1) return 11;
    return 8;
}

int collisions_update(Game *g) {
    Player *p  = &g->player;
    Fleet  *f  = &g->fleet;
    UFO    *u  = &g->ufo;
    int result = 0;

    /* 1. Player bullet vs invaders */
    if (p->bullet.active) {
        int bx = p->bullet.pos.x, by = p->bullet.pos.y;
        for (int r = 0; r < FLEET_ROWS && p->bullet.active; r++) {
            for (int c = 0; c < FLEET_COLS && p->bullet.active; c++) {
                Invader *inv = &f->grid[r][c];
                if (!inv->alive) continue;
                int iw = inv_w(inv->type);
                if (aabb_overlap(bx, by, BULLET_W, BULLET_H,
                                 inv->pos.x, inv->pos.y, iw, 8)) {
                    inv->alive = 0;
                    f->alive_count--;
                    bullet_clear(&p->bullet);
                    /* Speed up fleet */
                    f->step_interval -= FLEET_STEP_INT_DELTA;
                    if (f->step_interval < FLEET_STEP_MIN_INT)
                        f->step_interval = FLEET_STEP_MIN_INT;
                    /* Score */
                    static const int pts[3] = { 10, 20, 30 };
                    g->score += pts[inv->type];
                    if (g->score > g->high_score)
                        g->high_score = g->score;
                    if (f->alive_count == 0) result = 2;
                }
            }
        }
    }

    /* 2. Player bullet vs UFO */
    if (p->bullet.active && u->active) {
        if (aabb_overlap(p->bullet.pos.x, p->bullet.pos.y, BULLET_W, BULLET_H,
                         u->pos.x, u->pos.y, UFO_W, UFO_H)) {
            g->score += u->points;
            if (g->score > g->high_score) g->high_score = g->score;
            u->active = 0;
            u->spawn_timer = UFO_SPAWN_MIN +
                (float)(g->shot_count % 23) / 23.0f * (UFO_SPAWN_MAX - UFO_SPAWN_MIN);
            bullet_clear(&p->bullet);
        }
    }

    /* 3. Player bullet vs bunkers */
    if (p->bullet.active) {
        for (int i = 0; i < BUNKER_COUNT; i++)
            bunker_damage_bullet(&g->bunkers[i], &p->bullet);
    }

    /* 4. Enemy bullets vs player */
    if (p->invincible_frames == 0) {
        for (int i = 0; i < ENEMY_BULLETS; i++) {
            Bullet *eb = &f->bullets[i];
            if (!eb->active) continue;
            if (aabb_overlap(eb->pos.x, eb->pos.y, BULLET_W, BULLET_H,
                             p->pos.x, p->pos.y, PLAYER_W, PLAYER_H)) {
                bullet_clear(eb);
                p->lives--;
                p->invincible_frames = INVINCIBLE_FRAMES;
                p->pos.x = (SCREEN_W - PLAYER_W) / 2;
                bullet_clear(&p->bullet);
                if (p->lives <= 0) result = 1;
            }
        }
    }

    /* 5. Enemy bullets vs bunkers */
    for (int i = 0; i < ENEMY_BULLETS; i++) {
        if (!f->bullets[i].active) continue;
        for (int j = 0; j < BUNKER_COUNT; j++)
            bunker_damage_bullet(&g->bunkers[j], &f->bullets[i]);
    }

    /* 6. Invaders vs bunkers (absorb on contact) */
    for (int r = 0; r < FLEET_ROWS; r++) {
        for (int c = 0; c < FLEET_COLS; c++) {
            const Invader *inv = &f->grid[r][c];
            if (!inv->alive) continue;
            for (int bi = 0; bi < BUNKER_COUNT; bi++) {
                Bunker *bk = &g->bunkers[bi];
                if (!aabb_overlap(inv->pos.x, inv->pos.y, inv_w(inv->type), 8,
                                  bk->origin.x, bk->origin.y, BUNKER_W, BUNKER_H))
                    continue;
                /* Destroy cells that overlap */
                for (int br = 0; br < BUNKER_ROWS; br++) {
                    for (int bc = 0; bc < BUNKER_COLS; bc++) {
                        if (!bk->cells[br][bc]) continue;
                        int cx = bk->origin.x + bc * BUNKER_CELL;
                        int cy = bk->origin.y + br * BUNKER_CELL;
                        if (aabb_overlap(inv->pos.x, inv->pos.y, inv_w(inv->type), 8,
                                         cx, cy, BUNKER_CELL, BUNKER_CELL))
                            bk->cells[br][bc] = 0;
                    }
                }
            }
        }
    }

    /* 7. Fleet reached player line → game over */
    if (result == 0) {
        for (int r = 0; r < FLEET_ROWS; r++) {
            for (int c = 0; c < FLEET_COLS; c++) {
                if (f->grid[r][c].alive &&
                    f->grid[r][c].pos.y + 8 >= p->pos.y) {
                    result = 1;
                    p->lives = 0;
                }
            }
        }
    }

    return result;
}

/* ===== renderer.c ===== */

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
            uint16_t color = (bits & (1u << (unsigned)col)) ? fg : bg;
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

/* ===== framebuffer.c ===== */

/* Vitis standalone (bare-metal) — Zynq-7000 / Zybo Z7-20, pipeline de video
   AXI VDMA + rgb2dvi (HDMI TX). Veja vivado/build_hdmi.tcl e vivado/LEIAME.txt.

   DIFERENCAS vs. o design antigo (CTRL_REG custom):
   - O FRAMEBUFFER E 32 bits por pixel (0x00RRGGBB / XRGB8888). E o formato que o
     AXI VDMA (c_m_axis_mm2s_tdata_width = 32) + Subset Converter esperam.
     O jogo continua usando RGB565 (uint16_t) na API publica; a conversao para
     XRGB8888 acontece aqui, em put_pixel/get_pixel.
   - NAO existe CTRL_REG. O double buffer e o vsync sao feitos pelo proprio
     AXI VDMA (2 frame stores) operando em MODO PARK: o ponteiro de leitura
     (PARK_PTR_REG[4:0]) seleciona qual frame a PL exibe; trocamos esse ponteiro
     no swap. O flag de fim-de-frame (MM2S_VDMASR bit 12) faz o papel de vsync.

   COERENCIA DE CACHE: a DDR e cacheavel no BSP standalone. As escritas de pixel
   ficam em L1/L2 e o VDMA le a DDR direto — veria dados velhos. Antes de cada
   swap, Xil_DCacheFlushRange() no buffer recem-desenhado. */

/* ── Framebuffers em DDR (32 bpp, lidos pelo AXI VDMA via porta AXI HP0) ─────
   Ajuste FRAMEBUFFER_BASE/VDMA_BASE/GPIO p/ o Address Editor do Vivado (ou use
   os XPAR_* de xparameters.h: XPAR_AXI_VDMA_0_BASEADDR). Os 2 frames precisam
   caber na DDR (Zybo Z7-20 = 1 GB), fora da regiao de codigo/heap/stack. */
#define FB_BPP            4u                                   /* XRGB8888       */
#define FRAMEBUFFER_BASE  0x20000000u
#define FRAMEBUFFER_SIZE  ((unsigned)SCREEN_W * SCREEN_H * FB_BPP) /* 1.228.800 B */

static uint32_t * const fb[2] = {
    (uint32_t *)FRAMEBUFFER_BASE,
    (uint32_t *)(FRAMEBUFFER_BASE + FRAMEBUFFER_SIZE),
};
static int active_fb;     /* frame EXIBIDO pelo VDMA; back buffer = 1-active_fb */

/* ── Registradores do AXI VDMA (canal MM2S) ─────────────────────────────────
   Offsets do mapa de registradores do AXI VDMA (PG020). Base padrao p/ a 1a
   VDMA na GP0 costuma ser 0x4300_0000 — confirme no Address Editor. */
#define VDMA_BASE                0x43000000u
#define VDMA_MM2S_VDMACR         0x00u   /* control                            */
#define VDMA_MM2S_VDMASR         0x04u   /* status                             */
#define VDMA_PARK_PTR_REG        0x28u   /* [4:0] RD frame ptr (park)          */
#define VDMA_MM2S_VSIZE          0x50u   /* linhas; escrever AQUI inicia       */
#define VDMA_MM2S_HSIZE          0x54u   /* bytes por linha                    */
#define VDMA_MM2S_FRMDLY_STRIDE  0x58u   /* [15:0] stride em bytes             */
#define VDMA_MM2S_START_ADDR1    0x5Cu   /* endereco do frame store 1          */
#define VDMA_MM2S_START_ADDR2    0x60u   /* endereco do frame store 2          */

#define VDMACR_RS                (1u << 0)   /* run/stop                       */
#define VDMACR_CIRCULAR          (1u << 1)   /* 1=circular, 0=park             */
#define VDMACR_RESET             (1u << 2)   /* soft reset (auto-limpa)        */
#define VDMASR_HALTED            (1u << 0)
#define VDMASR_FRMCNT_IRQ        (1u << 12)  /* fim de frame (write-1-clear)   */

static inline uint32_t vdma_rd(uint32_t off)            { return Xil_In32(VDMA_BASE + off); }
static inline void     vdma_wr(uint32_t off, uint32_t v){ Xil_Out32(VDMA_BASE + off, v);    }

/* RGB565 (uint16_t) → XRGB8888 (0x00RRGGBB). Expande cada canal replicando os
   bits mais significativos para preencher 8 bits. */
static inline uint32_t rgb565_to_xrgb(uint16_t c) {
    uint32_t r = (c >> 11) & 0x1Fu;
    uint32_t g = (c >>  5) & 0x3Fu;
    uint32_t b =  c        & 0x1Fu;
    r = (r << 3) | (r >> 2);
    g = (g << 2) | (g >> 4);
    b = (b << 3) | (b >> 2);
    return (r << 16) | (g << 8) | b;
}

/* XRGB8888 → RGB565 (inverso, para get_pixel). */
static inline uint16_t xrgb_to_rgb565(uint32_t c) {
    uint32_t r = (c >> 16) & 0xFFu;
    uint32_t g = (c >>  8) & 0xFFu;
    uint32_t b =  c        & 0xFFu;
    return (uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

void fb_init(void) {
    active_fb = 0;

    /* Soft reset do canal MM2S e espera concluir. */
    vdma_wr(VDMA_MM2S_VDMACR, VDMACR_RESET);
    while (vdma_rd(VDMA_MM2S_VDMACR) & VDMACR_RESET) { }

    /* Enderecos dos 2 frame stores. */
    vdma_wr(VDMA_MM2S_START_ADDR1, FRAMEBUFFER_BASE);
    vdma_wr(VDMA_MM2S_START_ADDR2, FRAMEBUFFER_BASE + FRAMEBUFFER_SIZE);

    /* Modo PARK (CIRCULAR=0): exibe sempre o frame apontado por PARK_PTR_REG.
       RS=1 inicia o canal. */
    vdma_wr(VDMA_MM2S_VDMACR, VDMACR_RS);

    /* Park no frame 0 (= active_fb). */
    vdma_wr(VDMA_PARK_PTR_REG, 0u);

    /* Geometria do frame. VSIZE deve ser escrito por ULTIMO (dispara o fetch). */
    vdma_wr(VDMA_MM2S_FRMDLY_STRIDE, (unsigned)SCREEN_W * FB_BPP);  /* stride */
    vdma_wr(VDMA_MM2S_HSIZE,         (unsigned)SCREEN_W * FB_BPP);  /* bytes/linha */
    vdma_wr(VDMA_MM2S_VSIZE,         (unsigned)SCREEN_H);           /* linhas → start */
}

void fb_cleanup(void) {
    /* Para o canal MM2S do VDMA. */
    vdma_wr(VDMA_MM2S_VDMACR, vdma_rd(VDMA_MM2S_VDMACR) & ~VDMACR_RS);
}

void fb_swap(void) {
    int back = 1 - active_fb;

    /* Garante que o back buffer recem-desenhado esta na DDR antes do VDMA ler. */
    Xil_DCacheFlushRange((INTPTR)fb[back], FRAMEBUFFER_SIZE);

    /* Aponta o park ptr para o back buffer; a PL passa a exibi-lo no proximo
       limite de frame. */
    vdma_wr(VDMA_PARK_PTR_REG, (uint32_t)back);
    active_fb = back;

    /* "vsync": espera o VDMA completar um frame (bit FrmCnt, write-1-clear).
       Limpa o flag e espera ele subir de novo = um limite de frame passou. */
    vdma_wr(VDMA_MM2S_VDMASR, VDMASR_FRMCNT_IRQ);
    while (!(vdma_rd(VDMA_MM2S_VDMASR) & VDMASR_FRMCNT_IRQ)) { }
}

void put_pixel(int x, int y, uint16_t color) {
    if ((unsigned)x >= (unsigned)SCREEN_W || (unsigned)y >= (unsigned)SCREEN_H)
        return;
    fb[1 - active_fb][(unsigned)y * SCREEN_W + (unsigned)x] = rgb565_to_xrgb(color);
}

uint16_t get_pixel(int x, int y) {
    if ((unsigned)x >= (unsigned)SCREEN_W || (unsigned)y >= (unsigned)SCREEN_H)
        return COLOR_BLACK;
    return xrgb_to_rgb565(fb[1 - active_fb][(unsigned)y * SCREEN_W + (unsigned)x]);
}

/* ===== timer.c ===== */

/* Vitis standalone — usa o Global Timer 64-bit do Cortex-A9 via a API do BSP.
   COUNTS_PER_SECOND (xtime_l.h) ja vem com a frequencia correta do PS,
   entao nao dependemos de CPU_FREQ_HZ hardcoded. usleep() vem de sleep.h. */

void timer_init(void) {
    /* O BSP standalone ja inicializa o Global Timer no boot. */
}

uint64_t timer_us(void) {
    XTime t;
    XTime_GetTime(&t);
    return (uint64_t)t / (COUNTS_PER_SECOND / 1000000ULL);
}

void timer_usleep(uint32_t us) {
    usleep(us);
}

/* ===== input.c ===== */

/* Vitis standalone — Xilinx AXI GPIO (PL) na AXI GP0.
   Ajuste GPIO_PIO_BASE para o Address Editor do Vivado (ou troque por XPAR_*).
   Mapa AXI GPIO: +0x00 = GPIO_DATA, +0x04 = GPIO_TRI (tri-state, 1 = entrada). */

#define GPIO_PIO_BASE  0x41200000u
#define GPIO_DATA_OFF  0x00u
#define GPIO_TRI_OFF   0x04u

/* Botoes Zybo Z7 sao ativos-ALTO (pressionado = 1): BTN0=esq, BTN1=dir,
   BTN2=fogo, BTN3=pause */
#define BTN_LEFT   (1u << 0)
#define BTN_RIGHT  (1u << 1)
#define BTN_FIRE   (1u << 2)
#define BTN_PAUSE  (1u << 3)
#define BTN_MASK   0xFu

static uint32_t prev_raw;

int input_init(void) {
    Xil_Out32(GPIO_PIO_BASE + GPIO_TRI_OFF, BTN_MASK);   /* 1 = entrada */
    prev_raw = Xil_In32(GPIO_PIO_BASE + GPIO_DATA_OFF) & BTN_MASK;
    return 0;                   /* sem file descriptor em bare-metal */
}

void input_poll(int fd, InputState *state) {
    (void)fd;

    uint32_t raw          = Xil_In32(GPIO_PIO_BASE + GPIO_DATA_OFF) & BTN_MASK;
    uint32_t pressed      = raw      & BTN_MASK;  /* ativo-alto: 1 = pressionado */
    uint32_t prev_pressed = prev_raw & BTN_MASK;
    uint32_t just_pressed = pressed & ~prev_pressed; /* borda de subida */

    state->left  = (pressed      & BTN_LEFT)  ? 1 : 0;
    state->right = (pressed      & BTN_RIGHT) ? 1 : 0;
    state->fire  = (just_pressed & BTN_FIRE)  ? 1 : 0;
    state->pause = (just_pressed & BTN_PAUSE) ? 1 : 0;
    state->quit  = 0;
    state->reset = 0;

    prev_raw = raw;
}

void input_cleanup(int fd) {
    (void)fd;
}

/* ===== game.c ===== */

/* ── HUD layout ─────────────────────────────────────────────────────────── */
#define HUD_Y       456
#define HUD_BG      COLOR_BLACK
#define HUD_FG      COLOR_WHITE
#define HUD_LIVES_COLOR COLOR_GREEN

static void render_hud(const Game *g) {
    char buf[64];

    /* Background strip */
    draw_rect(0, HUD_Y - 2, SCREEN_W, 2, COLOR_GREEN);   /* separator line */
    draw_rect(0, HUD_Y, SCREEN_W, SCREEN_H - HUD_Y, COLOR_BLACK);

    snprintf(buf, sizeof(buf), "SCORE:%06d  HI:%06d  LV:%02d",
             g->score, g->high_score, g->level);
    draw_string(4, HUD_Y, buf, HUD_FG, HUD_BG);

    /* Lives as player sprites */
    draw_string(4, HUD_Y + 10, "LIVES:", HUD_LIVES_COLOR, HUD_BG);
    for (int i = 0; i < g->player.lives && i < 5; i++)
        draw_sprite(52 + i * 16, HUD_Y + 10, &SPRITE_PLAYER);
}

static void render_bunkers(const Game *g) {
    for (int bi = 0; bi < BUNKER_COUNT; bi++) {
        const Bunker *bk = &g->bunkers[bi];
        for (int r = 0; r < BUNKER_ROWS; r++) {
            for (int c = 0; c < BUNKER_COLS; c++) {
                if (!bk->cells[r][c]) continue;
                draw_rect(bk->origin.x + c * BUNKER_CELL,
                          bk->origin.y + r * BUNKER_CELL,
                          BUNKER_CELL, BUNKER_CELL, COLOR_GREEN);
            }
        }
    }
}

static void render_fleet(const Fleet *f) {
    for (int r = 0; r < FLEET_ROWS; r++) {
        for (int c = 0; c < FLEET_COLS; c++) {
            const Invader *inv = &f->grid[r][c];
            if (!inv->alive) continue;
            const Sprite *spr;
            int frame = inv->anim_frame;
            switch (inv->type) {
            case 0: spr = &SPRITE_INVADER_A[frame]; break;
            case 1: spr = &SPRITE_INVADER_B[frame]; break;
            default: spr = &SPRITE_INVADER_C[frame]; break;
            }
            draw_sprite(inv->pos.x, inv->pos.y, spr);
        }
    }
    /* Enemy bullets */
    for (int i = 0; i < ENEMY_BULLETS; i++) {
        const Bullet *b = &f->bullets[i];
        if (b->active)
            draw_sprite(b->pos.x, b->pos.y, &SPRITE_BULLET_ENEMY);
    }
}

static void render_player(const Player *p) {
    /* Blink while invincible */
    if (p->invincible_frames > 0 && (p->invincible_frames & 4))
        return;
    draw_sprite(p->pos.x, p->pos.y, &SPRITE_PLAYER);
    if (p->bullet.active)
        draw_sprite(p->bullet.pos.x, p->bullet.pos.y, &SPRITE_BULLET_PLAYER);
}

static void render_ufo(const UFO *u) {
    if (u->active)
        draw_sprite(u->pos.x, u->pos.y, &SPRITE_UFO);
}

/* ── Screen renderers ───────────────────────────────────────────────────── */
static void render_title(void) {
    clear_screen(COLOR_BLACK);
    draw_string(160, 140, "SPACE  INVADERS", COLOR_GREEN, COLOR_BLACK);
    draw_string(168, 180, "PRESS SPACE TO START", COLOR_WHITE, COLOR_BLACK);
    draw_string(192, 220, "Q / ESC TO QUIT", COLOR_WHITE, COLOR_BLACK);

    draw_string(152, 280, "SCORE TABLE", COLOR_YELLOW, COLOR_BLACK);
    draw_sprite(152, 300, &SPRITE_INVADER_C[0]);
    draw_string(176, 300, "= 30 PTS", COLOR_WHITE, COLOR_BLACK);
    draw_sprite(152, 316, &SPRITE_INVADER_B[0]);
    draw_string(176, 316, "= 20 PTS", COLOR_WHITE, COLOR_BLACK);
    draw_sprite(152, 332, &SPRITE_INVADER_A[0]);
    draw_string(176, 332, "= 10 PTS", COLOR_WHITE, COLOR_BLACK);
    draw_sprite(152, 348, &SPRITE_UFO);
    draw_string(176, 348, "= ???  PTS", COLOR_MAGENTA, COLOR_BLACK);
}

static void render_playing(const Game *g) {
    clear_screen(COLOR_BLACK);
    render_bunkers(g);
    render_fleet(&g->fleet);
    render_player(&g->player);
    render_ufo(&g->ufo);
    render_hud(g);
}

static void render_paused(const Game *g) {
    render_playing(g);
    draw_string(264, 220, "PAUSED", COLOR_YELLOW, COLOR_BLACK);
    draw_string(208, 236, "PRESS P TO RESUME", COLOR_WHITE, COLOR_BLACK);
}

static void render_next_level(const Game *g) {
    clear_screen(COLOR_BLACK);
    char buf[32];
    snprintf(buf, sizeof(buf), "LEVEL  %02d", g->level);
    draw_string(248, 220, buf, COLOR_GREEN, COLOR_BLACK);
    draw_string(208, 240, "GET READY!", COLOR_WHITE, COLOR_BLACK);
    render_hud(g);
}

static void render_game_over(const Game *g) {
    clear_screen(COLOR_BLACK);
    draw_string(232, 200, "GAME  OVER", COLOR_RED, COLOR_BLACK);
    char buf[32];
    snprintf(buf, sizeof(buf), "SCORE: %06d", g->score);
    draw_string(240, 224, buf, COLOR_WHITE, COLOR_BLACK);
    draw_string(184, 256, "PRESS R TO RESTART", COLOR_WHITE, COLOR_BLACK);
    draw_string(192, 272, "Q / ESC TO QUIT", COLOR_WHITE, COLOR_BLACK);
    render_hud(g);
}

/* ── Public API ─────────────────────────────────────────────────────────── */
void game_init(Game *g) {
    memset(g, 0, sizeof(*g));
    g->level      = 1;
    g->high_score = 0;
    g->state      = STATE_TITLE;
    g->input_fd   = -1;
}

static void start_level(Game *g) {
    player_init(&g->player);
    fleet_init(&g->fleet, g->level);
    ufo_init(&g->ufo);
    bunkers_init(g->bunkers);
    g->shot_count = 0;
}

void game_update(Game *g, float dt) {
    InputState *in = &g->input;

    switch (g->state) {
    case STATE_TITLE:
        if (in->fire || in->reset) {
            g->score = 0;
            g->level = 1;
            start_level(g);
            g->state = STATE_PLAYING;
        }
        if (in->quit) g->state = STATE_QUIT;
        break;

    case STATE_PLAYING:
        if (in->quit)  { g->state = STATE_QUIT; break; }
        if (in->pause) { g->state = STATE_PAUSED; break; }

        player_update(&g->player, in, &g->shot_count);
        fleet_update(&g->fleet, dt);
        ufo_update(&g->ufo, dt);

        {
            int r = collisions_update(g);
            if (r == 1) {
                /* Player killed */
                if (g->player.lives <= 0) {
                    g->state       = STATE_GAME_OVER;
                    g->state_timer = 0.0f;
                }
            } else if (r == 2) {
                /* Level cleared */
                g->level++;
                g->state       = STATE_NEXT_LEVEL;
                g->state_timer = 3.0f;
            }
        }
        break;

    case STATE_PAUSED:
        if (in->quit)  { g->state = STATE_QUIT; break; }
        if (in->pause) { g->state = STATE_PLAYING; break; }
        break;

    case STATE_NEXT_LEVEL:
        g->state_timer -= dt;
        if (g->state_timer <= 0.0f) {
            start_level(g);
            g->state = STATE_PLAYING;
        }
        break;

    case STATE_GAME_OVER:
        g->state_timer += dt;
        if (in->reset && g->state_timer > 1.0f) {
            g->score = 0;
            g->level = 1;
            start_level(g);
            g->state = STATE_PLAYING;
        }
        if (in->quit) g->state = STATE_QUIT;
        break;

    case STATE_QUIT:
        break;
    }
}

void game_render(const Game *g) {
    switch (g->state) {
    case STATE_TITLE:      render_title();        break;
    case STATE_PLAYING:    render_playing(g);     break;
    case STATE_PAUSED:     render_paused(g);      break;
    case STATE_NEXT_LEVEL: render_next_level(g);  break;
    case STATE_GAME_OVER:  render_game_over(g);   break;
    default:               break;
    }
}

/* ===== main.c ===== */

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

