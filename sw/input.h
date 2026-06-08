#pragma once
#include "game.h"

int  input_init(void);
void input_poll(int fd, InputState *state);
void input_cleanup(int fd);
