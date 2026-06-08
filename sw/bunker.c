#include "bunker.h"
#include "renderer.h"

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
