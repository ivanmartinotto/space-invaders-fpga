#include "collision.h"
#include "bunker.h"
#include "bullet.h"

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
