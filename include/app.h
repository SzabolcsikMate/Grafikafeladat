#ifndef APP_H
#define APP_H

#include <SDL2/SDL.h>
#include "game.h"

typedef struct App {
    SDL_Window* window;
    SDL_GLContext gl_context;
    GameState game;
    int running;
    int width;
    int height;
} App;

int init_app(App* app);
void run_app(App* app);
void destroy_app(App* app);

#endif