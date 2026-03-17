#include <math.h>
#include <SDL2/SDL.h>
#include "game.h"

static const float MOUSE_SENSITIVITY = 0.08f;

static void add_collider(GameState* game, Vec3 min, Vec3 max)
{
    if (game->collider_count >= MAX_COLLIDERS) {
        return;
    }

    game->colliders[game->collider_count].min = min;
    game->colliders[game->collider_count].max = max;
    game->collider_count++;
}

static void set_active_light(GameState* game, int index)
{
    int i;

    if (game->light_point_count <= 0) {
        return;
    }

    for (i = 0; i < game->light_point_count; i++) {
        game->light_points[i].active = 0;
    }

    game->current_target = index % game->light_point_count;
    game->light_points[game->current_target].active = 1;
}

static int can_move_to(GameState* game, Vec3 new_pos)
{
    int i;

    for (i = 0; i < game->collider_count; i++) {
        if (sphere_aabb_intersect(new_pos, game->player.radius, game->colliders[i])) {
            return 0;
        }
    }

    return 1;
}

void toggle_help(void)
{
    SDL_ShowSimpleMessageBox(
        SDL_MESSAGEBOX_INFORMATION,
        "Dark Museum - Help",
        "W A S D: move\n"
        "Mouse: look around\n"
        "Shift: faster movement\n"
        "F1: help\n"
        "+ / -: change active light strength\n"
        "R / Enter: restart after game over\n"
        "ESC: quit\n\n"
        "Reach the glowing light before your darkness timer runs out.\n"
        "If you stay too long in the dark, the creature catches you.",
        NULL
    );
}

void init_game(GameState* game)
{
    reset_game(game);
}

void reset_game(GameState* game)
{
    game->player.position = vec3(0.0f, 1.0f, 0.0f);
    game->player.yaw = 0.0f;
    game->player.pitch = 0.0f;
    game->player.radius = 0.35f;
    game->player.move_speed = 3.5f;

    game->collider_count = 0;
    game->light_point_count = 0;
    game->current_target = 0;
    game->darkness_limit = 12.0f;
    game->darkness_timer = game->darkness_limit;
    game->active_light_strength = 1.8f;
    game->game_over = 0;
    game->win_counter = 0;

    // New museum layout
    float size = 15.0f;
    float wall_thickness = 0.5f;
    float wall_height = 3.0f;

    // Outer walls
    add_collider(game, vec3(-size, 0.0f, -size), vec3(size, wall_height, -size + wall_thickness));
    add_collider(game, vec3(-size, 0.0f, size - wall_thickness), vec3(size, wall_height, size));
    add_collider(game, vec3(-size, 0.0f, -size), vec3(-size + wall_thickness, wall_height, size));
    add_collider(game, vec3(size - wall_thickness, 0.0f, -size), vec3(size, wall_height, size));

    // Internal walls forming a cross shape, creating 4 galleries
    // Vertical walls
    add_collider(game, vec3(-2.0f, 0.0f, -size + wall_thickness), vec3(-1.5f, wall_height, -4.0f));
    add_collider(game, vec3(1.5f, 0.0f, -size + wall_thickness), vec3(2.0f, wall_height, -4.0f));
    add_collider(game, vec3(-2.0f, 0.0f, 4.0f), vec3(-1.5f, wall_height, size - wall_thickness));
    add_collider(game, vec3(1.5f, 0.0f, 4.0f), vec3(2.0f, wall_height, size - wall_thickness));

    // Horizontal walls
    add_collider(game, vec3(-size + wall_thickness, 0.0f, -2.0f), vec3(-4.0f, wall_height, -1.5f));
    add_collider(game, vec3(4.0f, 0.0f, -2.0f), vec3(size - wall_thickness, wall_height, -1.5f));
    add_collider(game, vec3(-size + wall_thickness, 0.0f, 1.5f), vec3(-4.0f, wall_height, 2.0f));
    add_collider(game, vec3(4.0f, 0.0f, 1.5f), vec3(size - wall_thickness, wall_height, 2.0f));

    // Lights
    game->light_points[0].position = vec3(0.0f, 1.4f, 0.0f); // Central hall
    game->light_points[0].base_intensity = 1.5f;

    game->light_points[1].position = vec3(-10.0f, 1.4f, -10.0f); // Top-left gallery
    game->light_points[1].base_intensity = 1.5f;

    game->light_points[2].position = vec3(10.0f, 1.4f, -10.0f); // Top-right gallery
    game->light_points[2].base_intensity = 1.5f;

    game->light_points[3].position = vec3(-10.0f, 1.4f, 10.0f); // Bottom-left gallery
    game->light_points[3].base_intensity = 1.5f;

    game->light_points[4].position = vec3(10.0f, 1.4f, 10.0f); // Bottom-right gallery
    game->light_points[4].base_intensity = 1.5f;

    game->light_point_count = 5;
    set_active_light(game, 0);
}

static void handle_mouse_look(GameState* game, int mouse_dx, int mouse_dy)
{
    game->player.yaw -= mouse_dx * MOUSE_SENSITIVITY;
    game->player.pitch -= mouse_dy * MOUSE_SENSITIVITY;

    if (game->player.pitch > 89.0f) {
        game->player.pitch = 89.0f;
    }

    if (game->player.pitch < -89.0f) {
        game->player.pitch = -89.0f;
    }
}

static Vec3 get_forward(const GameState* game)
{
    float yaw_rad = game->player.yaw * (float)M_PI / 180.0f;
    return vec3(-sinf(yaw_rad), 0.0f, -cosf(yaw_rad));
}

static Vec3 get_right(const GameState* game)
{
    float yaw_rad = game->player.yaw * (float)M_PI / 180.0f;
    return vec3(cosf(yaw_rad), 0.0f, -sinf(yaw_rad));
}

static void handle_movement(GameState* game, float dt, const unsigned char* key_state)
{
    Vec3 move_dir = vec3(0.0f, 0.0f, 0.0f);
    Vec3 forward = get_forward(game);
    Vec3 right = get_right(game);
    float speed = game->player.move_speed;

    if (key_state[SDL_SCANCODE_LSHIFT] || key_state[SDL_SCANCODE_RSHIFT]) {
        speed = 5.4f;
    }

    if (key_state[SDL_SCANCODE_W]) {
        move_dir = vec3_add(move_dir, forward);
    }

    if (key_state[SDL_SCANCODE_S]) {
        move_dir = vec3_sub(move_dir, forward);
    }

    if (key_state[SDL_SCANCODE_A]) {
        move_dir = vec3_sub(move_dir, right);
    }

    if (key_state[SDL_SCANCODE_D]) {
        move_dir = vec3_add(move_dir, right);
    }

    if (vec3_length(move_dir) > 0.0001f) {
        Vec3 step = vec3_scale(vec3_normalize(move_dir), speed * dt);
        Vec3 try_x = game->player.position;
        Vec3 try_z = game->player.position;

        try_x.x += step.x;
        if (can_move_to(game, try_x)) {
            game->player.position.x = try_x.x;
        }

        try_z.z += step.z;
        if (can_move_to(game, try_z)) {
            game->player.position.z = try_z.z;
        }
    }
}

static void update_darkness(GameState* game, float dt)
{
    if (game->light_point_count == 0) return;
    LightPoint* target = &game->light_points[game->current_target];
    Vec3 to_target = vec3_sub(target->position, game->player.position);
    float dist = vec3_length(to_target);

    if (dist < 1.35f) {
        game->darkness_timer = game->darkness_limit;
        game->win_counter++;
        set_active_light(game, (game->current_target + 1) % game->light_point_count);
    } else {
        game->darkness_timer -= dt;
        if (game->darkness_timer <= 0.0f) {
            game->darkness_timer = 0.0f;
            game->game_over = 1;

            SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_WARNING,
                "Game Over",
                "The darkness caught you.\n\nPress R or Enter to restart.",
                NULL
            );
        }
    }
}

void update_game(GameState* game, float dt, const unsigned char* key_state, int mouse_dx, int mouse_dy, int* quit_requested)
{
    if (key_state[SDL_SCANCODE_ESCAPE]) {
        *quit_requested = 1;
        return;
    }

    if (game->game_over) {
        if (key_state[SDL_SCANCODE_R] || key_state[SDL_SCANCODE_RETURN]) {
            reset_game(game);
        }
        return;
    }

    handle_mouse_look(game, mouse_dx, mouse_dy);
    handle_movement(game, dt, key_state);
    update_darkness(game, dt);

    if (key_state[SDL_SCANCODE_EQUALS] || key_state[SDL_SCANCODE_KP_PLUS]) {
        game->active_light_strength += 0.8f * dt;
    }

    if (key_state[SDL_SCANCODE_MINUS] || key_state[SDL_SCANCODE_KP_MINUS]) {
        game->active_light_strength -= 0.8f * dt;
    }

    if (game->active_light_strength < 0.6f) {
        game->active_light_strength = 0.6f;
    }

    if (game->active_light_strength > 3.2f) {
        game->active_light_strength = 3.2f;
    }
}