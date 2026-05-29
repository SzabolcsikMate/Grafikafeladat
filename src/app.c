#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "app.h"
#include "render.h"
#include "ui.h"
#include "texture.h"
#include "model.h"

static int is_inside_button(float x, float y, float x1, float y1, float x2, float y2)
{
    return x >= x1 && x <= x2 && y >= y1 && y <= y2;
}

static void enter_game(App* app)
{
    app->in_menu = 0;

    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_GetRelativeMouseState(NULL, NULL);

    app->game.inverted_mouse = app->inverted_mouse;
}

static void enter_menu(App* app)
{
    app->in_menu = 1;

    reset_game(&app->game);
    app->game.inverted_mouse = app->inverted_mouse;

    SDL_SetRelativeMouseMode(SDL_FALSE);
    SDL_ShowCursor(SDL_ENABLE);
    SDL_GetRelativeMouseState(NULL, NULL);
}

static void restart_game(App* app)
{
    reset_game(&app->game);
    app->game.inverted_mouse = app->inverted_mouse;

    app->in_menu = 0;

    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_GetRelativeMouseState(NULL, NULL);
}

int init_app(App* app)
{
    app->window = NULL;
    app->gl_context = NULL;
    app->running = 1;
    app->width = 1280;
    app->height = 720;

    app->lamp_model = 0;
    app->floor_texture = 0;
    app->wall_texture = 0;
    app->ceiling_texture = 0;

    app->in_menu = 1;
    app->fullscreen = 0;
    app->inverted_mouse = 0;

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

    SDL_SetRelativeMouseMode(SDL_FALSE);
    SDL_ShowCursor(SDL_ENABLE);

    init_render_state();
    resize_viewport(app->width, app->height);
    init_game(&app->game);

    app->game.inverted_mouse = app->inverted_mouse;

    app->floor_texture = load_texture_bmp("assets/textures/floor.bmp");
    app->wall_texture = load_texture_bmp("assets/textures/wall.bmp");
    app->ceiling_texture = load_texture_bmp("assets/textures/ceiling.bmp");

    if (!app->floor_texture) {
        fprintf(stderr, "Warning: floor texture not loaded.\n");
    }

    if (!app->wall_texture) {
        fprintf(stderr, "Warning: wall texture not loaded.\n");
    }

    if (!app->ceiling_texture) {
        fprintf(stderr, "Warning: ceiling texture not loaded.\n");
    }

    app->lamp_model = load_glb_model("assets/models/lamp.glb");

    if (!app->lamp_model) {
        app->lamp_model = load_glb_model("../assets/models/lamp.glb");
    }

    if (!app->lamp_model) {
        fprintf(stderr, "Warning: lamp model not loaded.\n");
    }

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
            }

            else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    app->width = event.window.data1;
                    app->height = event.window.data2;
                    resize_viewport(app->width, app->height);
                }
            }

            else if (event.type == SDL_MOUSEMOTION) {
                if (!app->in_menu && !app->game.game_over && !app->game.escaped) {
                    mouse_dx += event.motion.xrel;
                    mouse_dy += event.motion.yrel;

                    if (mouse_dx > 80) mouse_dx = 80;
                    if (mouse_dx < -80) mouse_dx = -80;
                    if (mouse_dy > 80) mouse_dy = 80;
                    if (mouse_dy < -80) mouse_dy = -80;
                }
            }

            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    if (app->in_menu) {
                        app->running = 0;
                    } else if (app->game.game_over || app->game.escaped) {
                        SDL_SetRelativeMouseMode(SDL_FALSE);
                        SDL_ShowCursor(SDL_ENABLE);
                    } else {
                        app->in_menu = 1;
                        SDL_SetRelativeMouseMode(SDL_FALSE);
                        SDL_ShowCursor(SDL_ENABLE);
                        SDL_GetRelativeMouseState(NULL, NULL);
                    }
                }

                if (!app->in_menu &&
                    !app->game.game_over &&
                    !app->game.escaped &&
                    event.key.keysym.sym == SDLK_F1) {
                    toggle_help();
                }
            }

            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    int mx = event.button.x;
                    int my = event.button.y;

                    float x = (float)mx / (float)app->width;
                    float y = 1.0f - (float)my / (float)app->height;

                    if (!app->in_menu && (app->game.game_over || app->game.escaped)) {
                        if (is_inside_button(x, y, 0.34f, 0.46f, 0.66f, 0.55f)) {
                            restart_game(app);
                        }
                        else if (is_inside_button(x, y, 0.34f, 0.33f, 0.66f, 0.42f)) {
                            enter_menu(app);
                        }
                        else if (is_inside_button(x, y, 0.34f, 0.20f, 0.66f, 0.29f)) {
                            app->running = 0;
                        }
                    }

                    else if (app->in_menu) {
                        if (is_inside_button(x, y, 0.34f, 0.56f, 0.66f, 0.65f)) {
                            enter_game(app);
                        }
                        else if (is_inside_button(x, y, 0.34f, 0.44f, 0.66f, 0.53f)) {
                            app->fullscreen = !app->fullscreen;

                            SDL_SetWindowFullscreen(
                                app->window,
                                app->fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0
                            );
                        }
                        else if (is_inside_button(x, y, 0.34f, 0.32f, 0.66f, 0.41f)) {
                            app->inverted_mouse = !app->inverted_mouse;
                            app->game.inverted_mouse = app->inverted_mouse;
                        }
                        else if (is_inside_button(x, y, 0.34f, 0.20f, 0.66f, 0.29f)) {
                            app->running = 0;
                        }
                    }
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

        if (app->in_menu) {
            SDL_SetRelativeMouseMode(SDL_FALSE);
            SDL_ShowCursor(SDL_ENABLE);

            render_menu(
                app->window,
                app->fullscreen,
                app->inverted_mouse
            );
        } else {
            if (app->game.game_over || app->game.escaped || app->game.dying) {
                SDL_SetRelativeMouseMode(SDL_FALSE);
                SDL_ShowCursor(SDL_ENABLE);
                mouse_dx = 0;
                mouse_dy = 0;
            } else {
                SDL_SetRelativeMouseMode(SDL_TRUE);
                SDL_ShowCursor(SDL_DISABLE);
            }

            app->game.inverted_mouse = app->inverted_mouse;

            update_game(
                &app->game,
                dt,
                key_state,
                mouse_dx,
                mouse_dy,
                &app->running
            );

            render_scene(
                app->window,
                &app->game,
                app->floor_texture,
                app->wall_texture,
                app->ceiling_texture,
                app->lamp_model
            );
        }

        SDL_GL_SwapWindow(app->window);
    }
}

void destroy_app(App* app)
{
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

    if (app->lamp_model) {
        glDeleteLists(app->lamp_model, 1);
        app->lamp_model = 0;
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