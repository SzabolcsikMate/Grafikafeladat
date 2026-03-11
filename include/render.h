#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include <GL/gl.h>
#include "game.h"

void init_render_state(void);
void resize_viewport(int width, int height);
void render_scene(SDL_Window* window, const GameState* game, GLuint floor_texture, GLuint wall_texture, GLuint ceiling_texture);

#endif