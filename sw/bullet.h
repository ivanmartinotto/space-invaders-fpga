#pragma once
#include "game.h"

void bullet_update(Bullet *b);
void bullet_fire(Bullet *b, int x, int y, int dx, int dy);
void bullet_clear(Bullet *b);
