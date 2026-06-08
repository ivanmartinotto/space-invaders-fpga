#include "invaders.h"
#include "bullet.h"
#include <stdlib.h>

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
