#include "player.h"
#include "bullet.h"

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
