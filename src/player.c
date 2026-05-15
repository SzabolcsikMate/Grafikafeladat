#include <math.h>
#include <SDL2/SDL.h>
#include "player.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MOUSE_SENSITIVITY 0.08f
#define GRAVITY -16.0f
#define JUMP_FORCE 6.8f

static int can_move_to(GameState* game, Vec3 new_pos);

static void handle_mouse_look(GameState* game, int mouse_dx, int mouse_dy)
{
    game->player.yaw -= mouse_dx * MOUSE_SENSITIVITY;
    if (game->inverted_mouse) {
        game->player.pitch += mouse_dy * MOUSE_SENSITIVITY;
    } else {
        game->player.pitch -= mouse_dy * MOUSE_SENSITIVITY;
    }

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
    Vec3 step;
    Vec3 try_pos;
    float ground_y;
    int i;

    if (key_state[SDL_SCANCODE_LSHIFT] || key_state[SDL_SCANCODE_RSHIFT]) {
        speed = 5.4f;
    }

    if (key_state[SDL_SCANCODE_W]) move_dir = vec3_add(move_dir, forward);
    if (key_state[SDL_SCANCODE_S]) move_dir = vec3_sub(move_dir, forward);
    if (key_state[SDL_SCANCODE_A]) move_dir = vec3_sub(move_dir, right);
    if (key_state[SDL_SCANCODE_D]) move_dir = vec3_add(move_dir, right);

    if (vec3_length(move_dir) > 0.0001f) {
        step = vec3_scale(vec3_normalize(move_dir), speed * dt);

        try_pos = game->player.position;
        try_pos.x += step.x;
        if (can_move_to(game, try_pos)) {
            game->player.position.x = try_pos.x;
        }

        try_pos = game->player.position;
        try_pos.z += step.z;
        if (can_move_to(game, try_pos)) {
            game->player.position.z = try_pos.z;
        }
    }

    if (key_state[SDL_SCANCODE_SPACE] && !game->player.is_jumping) {
        game->player.y_velocity = JUMP_FORCE;
        game->player.is_jumping = 1;
    }

    game->player.y_velocity += GRAVITY * dt;
    game->player.position.y += game->player.y_velocity * dt;

    ground_y = -100.0f;

    for (i = 0; i < game->collider_count; i++) {
        AABB box = game->colliders[i];

        if (game->player.position.x > box.min.x - game->player.radius &&
            game->player.position.x < box.max.x + game->player.radius &&
            game->player.position.z > box.min.z - game->player.radius &&
            game->player.position.z < box.max.z + game->player.radius) {

            if (game->player.position.y >= box.max.y &&
                game->player.y_velocity <= 0.0f) {
                if (box.max.y > ground_y) {
                    ground_y = box.max.y;
                }
            }
        }
    }

    if (ground_y > -50.0f &&
        game->player.position.y <= ground_y + PLAYER_HEIGHT) {
        game->player.position.y = ground_y + PLAYER_HEIGHT;
        game->player.y_velocity = 0.0f;
        game->player.is_jumping = 0;
    }

    if (game->player.position.y < -4.0f) {
        game->time_remaining = 0.0f;
    }
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

void update_player(GameState* game, float dt, const unsigned char* key_state, int mouse_dx, int mouse_dy)
{
    handle_mouse_look(game, mouse_dx, mouse_dy);
    handle_movement(game, dt, key_state);
}
