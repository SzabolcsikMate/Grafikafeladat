#ifndef LAMPS_H
#define LAMPS_H

#include <SDL2/SDL.h>
#include "game.h"

#define LAMP_MIN_POWER 0.08f
#define LAMP_MAX_POWER 1.20f

void add_light(GameState* game, Vec3 pos, float reach, float safe_radius);
void set_active_light(GameState* game, int index);
void update_light_interaction(GameState* game, float dt, const unsigned char* key_state);
int is_inside_any_safe_light(const GameState* game);
float distance_xz(Vec3 a, Vec3 b);

#endif
