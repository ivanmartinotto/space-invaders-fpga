#pragma once
#include "game.h"

void bunker_init(Bunker *b, int origin_x, int origin_y);
void bunkers_init(Bunker bunkers[BUNKER_COUNT]);
int  bunker_damage_bullet(Bunker *b, Bullet *bullet);
