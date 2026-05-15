#include <SDL2/SDL.h>
#include "game.h"
#include "map.h"
#include "lamps.h"
#include "player.h"

void init_game(GameState* game)
{
    reset_game(game);
}

void reset_game(GameState* game)
{
    game->inverted_mouse = 0;

    game->selected_light_index = -1;
    game->selected_light_ratio = 0.0f;

    game->player.position = vec3(0.0f, PLAYER_HEIGHT, 17.0f);
    game->player.yaw = 0.0f;
    game->player.pitch = 0.0f;
    game->player.radius = 0.35f;
    game->player.move_speed = 3.8f;
    game->player.y_velocity = 0.0f;
    game->player.is_jumping = 0;

    game->max_time = 7.0f;
    game->time_remaining = game->max_time;
    game->game_over_fade = 0.0f;
    game->game_over = 0;
    game->dying = 0;
    game->darkness_alpha = 0.0f;
    game->win_counter = 0;
    game->escaped = 0;
    game->end_screen = 0;

    game->exit_position = vec3(39.8f, 1.0f, -104.0f);
    game->exit_radius = 3.4f;



    build_level(game);

    if (game->light_point_count > 0) {
        set_active_light(game, 0);
        game->light_points[0].current_intensity = LAMP_MIN_POWER;
    }
}

void update_game(GameState* game, float dt, const unsigned char* key_state, int mouse_dx, int mouse_dy, int* quit_requested)
{
    if (game->escaped) {
        return;
    }

    if (game->game_over) {
        game->game_over_fade += dt * 0.70f;

        if (game->game_over_fade > 1.0f) {
            game->game_over_fade = 1.0f;
        }

        if (key_state[SDL_SCANCODE_R] || key_state[SDL_SCANCODE_RETURN]) {
            reset_game(game);
        }

        return;
    }

    if (game->dying) {
        game->time_remaining = 0.0f;
        game->darkness_alpha += dt * 0.45f;

        if (game->darkness_alpha >= 1.0f) {
            game->darkness_alpha = 1.0f;
            game->game_over = 1;
            game->game_over_fade = 1.0f;
        }

        return;
    }

    update_player(game, dt, key_state, mouse_dx, mouse_dy);
    update_light_interaction(game, dt, key_state);

    if (game->current_target >= 0 &&
        game->current_target < game->light_point_count &&
        game->light_points[game->current_target].collected) {
        if (game->current_target + 1 < game->light_point_count) {
            set_active_light(game, game->current_target + 1);
        }
        }

    if (game->win_counter >= 10 &&
        distance_xz(game->player.position, game->exit_position) <= game->exit_radius) {
        game->escaped = 1;
        game->end_screen = 1;
        return;
        }

    if (!is_inside_any_safe_light(game)) {
        game->time_remaining -= dt;

        if (game->time_remaining <= 0.0f) {
            game->time_remaining = 0.0f;
            game->dying = 1;
            game->darkness_alpha = 0.0f;
            return;
        }
    } else {
        if (game->time_remaining > 0.0f) {
            game->darkness_alpha = 0.0f;
        }
    }
}

void toggle_help(void)
{
    SDL_ShowSimpleMessageBox(
        SDL_MESSAGEBOX_INFORMATION,
        "Dark Museum - Help",
        "W A S D: move\n"
        "Mouse: look around\n"
        "SPACE: jump\n"
        "Shift: run\n"
        "E / + / UP: increase lamp power\n"
        "Q / - / DOWN: decrease lamp power\n"
        "F1: help\n"
        "R: restart after death\n"
        "ESC: quit\n\n"
        "Look at the active lamp from close range.\n"
        "When it is highlighted, use the power keys.\n"
        "A powered lamp creates a safe light zone.\n"
        "Falling in the parkour section kills you.",
        NULL
    );
}
