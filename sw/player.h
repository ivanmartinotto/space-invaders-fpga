#pragma once
#include "game.h"

void player_init(Player *p);
void player_update(Player *p, const InputState *input, int *shot_count);
