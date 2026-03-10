#include <stdio.h>
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include "../include/app.h"
#include "../include/render.h"

int init_app(App* app)
{
    app->window = NULL;
    app->gl_context = NULL;
    app->running = 1;
    app->width = 1280;
    app->height = 720;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL init failed: %s\n", SDL_GetError());
        return 0;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    app->window = SDL_CreateWindow(
        "Dark Museum",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        app->width,
        app->height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    if (!app->window) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    app->gl_context = SDL_GL_CreateContext(app->window);

    if (!app->gl_context) {
        fprintf(stderr, "OpenGL context creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(app->window);
        SDL_Quit();
        return 0;
    }

    SDL_GL_SetSwapInterval(1);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    init_render_state();
    resize_viewport(app->width, app->height);
    init_game(&app->game);

    return 1;
}

void run_app(App* app)
{
    Uint32 last_ticks = SDL_GetTicks();

    while (app->running) {
        SDL_Event event;
        Uint32 current_ticks;
        float dt;
        int mouse_dx = 0;
        int mouse_dy = 0;
        const unsigned char* key_state;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                app->running = 0;
            } else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    app->width = event.window.data1;
                    app->height = event.window.data2;
                    resize_viewport(app->width, app->height);
                }
            } else if (event.type == SDL_MOUSEMOTION) {
                mouse_dx += event.motion.xrel;
                mouse_dy += event.motion.yrel;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_F1) {
                    toggle_help();
                }
            }
        }

        current_ticks = SDL_GetTicks();
        dt = (float)(current_ticks - last_ticks) / 1000.0f;
        last_ticks = current_ticks;

        if (dt > 0.05f) {
            dt = 0.05f;
        }

        key_state = SDL_GetKeyboardState(NULL);
        update_game(&app->game, dt, key_state, mouse_dx, mouse_dy, &app->running);
        render_scene(app->window, &app->game);

        SDL_GL_SwapWindow(app->window);
    }
}

void destroy_app(App* app)
{
    if (app->gl_context) {
        SDL_GL_DeleteContext(app->gl_context);
    }

    if (app->window) {
        SDL_DestroyWindow(app->window);
    }

    SDL_Quit();
}