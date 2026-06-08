#include "game.h"
#include "player.h"
#include "invaders.h"
#include "ufo.h"
#include "bunker.h"
#include "bullet.h"
#include "collision.h"
#include "renderer.h"
#include "assets/sprites.h"
#include <stdio.h>
#include <string.h>

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
