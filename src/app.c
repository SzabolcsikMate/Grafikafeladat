#include <stdio.h>
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include "app.h"
#include "render.h"
#include "texture.h"

int init_app(App *app) {
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

    // Kérésnek megfelelően visszaállítva a régi biztonságos betöltési mód a ../ részekkel,
    // hogy a CLion futtatási környezetből (cmake-build-debug mappából) is biztosan menjen
    app->floor_texture = load_texture_bmp("../assets/textures/floor.bmp");
    if (!app->floor_texture) {
         app->floor_texture = load_texture_bmp("assets/textures/floor.bmp");
    }

    app->wall_texture = load_texture_bmp("../assets/textures/wall.bmp");
    if (!app->wall_texture) {
         app->wall_texture = load_texture_bmp("assets/textures/wall.bmp");
    }

    app->ceiling_texture = load_texture_bmp("../assets/textures/ceiling.bmp");
    if (!app->ceiling_texture) {
        app->ceiling_texture = load_texture_bmp("assets/textures/ceiling.bmp");
    }

    if (app->floor_texture == 0) {
        fprintf(stderr, "Warning: floor texture not loaded.\n");
    }

    if (app->wall_texture == 0) {
        fprintf(stderr, "Warning: wall texture not loaded.\n");
    }

    if (app->ceiling_texture == 0) {
        fprintf(stderr, "Warning: ceiling texture not loaded.\n");
    }

    return 1;
}

void run_app(App *app) {
    Uint32 last_ticks = SDL_GetTicks();
    const unsigned char *key_state;
    Uint32 current_ticks;
    float dt;
    int mouse_dx;
    int mouse_dy;
    SDL_Event event;

    while (app->running) {
        mouse_dx = 0;
        mouse_dy = 0;

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
                } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                    app->running = 0;
                }
            }
        }

        current_ticks = SDL_GetTicks();
        dt = (float) (current_ticks - last_ticks) / 1000.0f;
        last_ticks = current_ticks;

        if (dt > 0.05f) {
            dt = 0.05f;
        }

        key_state = SDL_GetKeyboardState(NULL);
        update_game(&app->game, dt, key_state, mouse_dx, mouse_dy, &app->running);

        render_scene(app->window, &app->game, app->floor_texture, app->wall_texture, app->ceiling_texture);

        SDL_GL_SwapWindow(app->window);
    }
}

void destroy_app(App *app) {
    if (app->floor_texture) {
        glDeleteTextures(1, &app->floor_texture);
        app->floor_texture = 0;
    }

    if (app->wall_texture) {
        glDeleteTextures(1, &app->wall_texture);
        app->wall_texture = 0;
    }

    if (app->ceiling_texture) {
        glDeleteTextures(1, &app->ceiling_texture);
        app->ceiling_texture = 0;
    }

    if (app->gl_context) {
        SDL_GL_DeleteContext(app->gl_context);
        app->gl_context = NULL;
    }

    if (app->window) {
        SDL_DestroyWindow(app->window);
        app->window = NULL;
    }

    SDL_Quit();
}