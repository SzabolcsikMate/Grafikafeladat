#ifndef UI_H
#define UI_H

#include <SDL2/SDL.h>
#include "game.h"

void render_menu(SDL_Window* window, int fullscreen, int inverted_mouse);
void render_game_ui(const GameState* game);

#endif
