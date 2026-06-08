#include "bullet.h"

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
