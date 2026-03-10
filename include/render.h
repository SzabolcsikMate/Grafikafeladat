#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include "game.h"

void init_render_state(void);
void resize_viewport(int width, int height);
void render_scene(SDL_Window* window, const GameState* game);

#endif