#include "ufo.h"
#include <stdlib.h>

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
