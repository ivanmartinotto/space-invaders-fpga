#pragma once
#include "game.h"

void fleet_init(Fleet *f, int level);
void fleet_update(Fleet *f, float dt);
int  fleet_alive_count(const Fleet *f);
