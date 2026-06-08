#pragma once
#include "game.h"

/* Returns 1 if the two axis-aligned boxes overlap */
int aabb_overlap(int ax, int ay, int aw, int ah,
                 int bx, int by, int bw, int bh);

/* Process all collisions for one frame. Returns:
     0 = nothing special
     1 = player killed (lives--; if lives==0 caller sets GAME_OVER)
     2 = all invaders cleared (caller advances level)  */
int collisions_update(Game *g);
