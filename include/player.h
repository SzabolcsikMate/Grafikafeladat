#ifndef PLAYER_H
#define PLAYER_H

#include <SDL2/SDL.h>
#include "game.h"

#define PLAYER_HEIGHT 1.0f

void update_player(GameState* game, float dt, const unsigned char* key_state, int mouse_dx, int mouse_dy);

#endif
