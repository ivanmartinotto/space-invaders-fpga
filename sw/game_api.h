#pragma once
#include "game.h"

void game_init(Game *g);
void game_update(Game *g, float dt);
void game_render(const Game *g);
