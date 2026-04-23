#include <math.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include "game.h"

static const float MOUSE_SENSITIVITY = 0.08f;
static const float GRAVITY = -15.0f;
static const float JUMP_FORCE = 6.0f;
static const float PLAYER_HEIGHT = 1.0f;

static void add_collider(GameState* game, Vec3 min, Vec3 max)
{
    if (game->collider_count >= MAX_COLLIDERS) {
        return;
    }

    game->colliders[game->collider_count].min = min;
    game->colliders[game->collider_count].max = max;
    game->collider_count++;
}

static void add_map_object(GameState* game, ObjectType type, Vec3 pos, Vec3 scale)
{
    Vec3 min;
    Vec3 max;

    if (game->object_count >= MAX_MAP_OBJECTS) {
        return;
    }

    game->map_objects[game->object_count].type = type;
    game->map_objects[game->object_count].position = pos;
    game->map_objects[game->object_count].scale = scale;
    game->object_count++;

    min = vec3(pos.x - scale.x * 0.5f, 0.0f, pos.z - scale.z * 0.5f);
    max = vec3(pos.x + scale.x * 0.5f, scale.y, pos.z + scale.z * 0.5f);
    add_collider(game, min, max);
}

static void add_light(GameState* game, Vec3 pos, float reach, float safe_radius)
{
    if (game->light_point_count >= MAX_LIGHT_POINTS) {
        return;
    }

    game->light_points[game->light_point_count].position = pos;
    game->light_points[game->light_point_count].current_intensity = 0.25f;
    game->light_points[game->light_point_count].active = 0;
    game->light_points[game->light_point_count].collected = 0;
    game->light_points[game->light_point_count].reach_radius = reach;
    game->light_points[game->light_point_count].safe_radius = safe_radius;
    game->light_point_count++;
}

static float distance_xz(Vec3 a, Vec3 b)
{
    float dx = a.x - b.x;
    float dz = a.z - b.z;
    return sqrtf(dx * dx + dz * dz);
}

static Vec3 get_forward_3d(const GameState* game)
{
    float yaw_rad = game->player.yaw * (float)M_PI / 180.0f;
    float pitch_rad = game->player.pitch * (float)M_PI / 180.0f;

    Vec3 forward;
    forward.x = -sinf(yaw_rad) * cosf(pitch_rad);
    forward.y = sinf(pitch_rad);
    forward.z = -cosf(yaw_rad) * cosf(pitch_rad);
    return vec3_normalize(forward);
}

static int is_looking_at_light(const GameState* game, const LightPoint* light)
{
    Vec3 forward = get_forward_3d(game);
    Vec3 to_light = vec3_sub(light->position, game->player.position);
    float len = vec3_length(to_light);
    float dot;

    if (len < 0.001f) {
        return 1;
    }

    to_light = vec3_scale(to_light, 1.0f / len);
    dot = forward.x * to_light.x + forward.y * to_light.y + forward.z * to_light.z;

    return dot > 0.85f;
}

static void set_active_light(GameState* game, int index)
{
    int i;

    for (i = 0; i < game->light_point_count; i++) {
        game->light_points[i].active = 0;
    }

    if (index >= 0 && index < game->light_point_count) {
        game->current_target = index;
        game->light_points[index].active = 1;
        if (!game->light_points[index].collected && game->light_points[index].current_intensity < 0.25f) {
            game->light_points[index].current_intensity = 0.25f;
        }
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

void toggle_help(void)
{
    SDL_ShowSimpleMessageBox(
        SDL_MESSAGEBOX_INFORMATION,
        "Dark Museum - Help",
        "W A S D: move\n"
        "SPACE: jump\n"
        "Mouse: look around\n"
        "Shift: run\n"
        "+ / -: increase or decrease the current nearby lamp\n"
        "F1: help\n"
        "R: restart after game over\n"
        "ESC: quit\n\n"
        "Stand close to the active lamp and look at it.\n"
        "Use + or - to change only that lamp.\n"
        "When a lamp becomes strong enough, it stays on.\n"
        "Standing inside active lamp light stops the timer.",
        NULL
    );
}

static void build_level(GameState* game)
{
    float wall_h = 3.0f;
    float wall_t = 0.5f;
    Vec3 col_scale = vec3(0.8f, wall_h, 0.8f);

    game->collider_count = 0;
    game->object_count = 0;
    game->light_point_count = 0;

    /* Külső falak */
    add_map_object(game, OBJ_BOUNDING_WALL, vec3(0.0f, 0.0f, -18.0f), vec3(36.0f, wall_h, wall_t));
    add_map_object(game, OBJ_BOUNDING_WALL, vec3(0.0f, 0.0f, 18.0f), vec3(36.0f, wall_h, wall_t));
    add_map_object(game, OBJ_BOUNDING_WALL, vec3(-18.0f, 0.0f, 0.0f), vec3(wall_t, wall_h, 36.0f));
    add_map_object(game, OBJ_BOUNDING_WALL, vec3(18.0f, 0.0f, 0.0f), vec3(wall_t, wall_h, 36.0f));

    /* Központi csarnok és folyosók */
    add_map_object(game, OBJ_WALL_PANEL, vec3(-6.0f, 0.0f, 8.0f), vec3(8.0f, wall_h, wall_t));
    add_map_object(game, OBJ_WALL_PANEL, vec3(6.0f, 0.0f, 8.0f), vec3(8.0f, wall_h, wall_t));

    add_map_object(game, OBJ_WALL_PANEL, vec3(-8.0f, 0.0f, 0.0f), vec3(wall_t, wall_h, 12.0f));
    add_map_object(game, OBJ_WALL_PANEL, vec3(8.0f, 0.0f, 0.0f), vec3(wall_t, wall_h, 12.0f));

    add_map_object(game, OBJ_WALL_PANEL, vec3(0.0f, 0.0f, -8.0f), vec3(10.0f, wall_h, wall_t));

    /* Bal szárny */
    add_map_object(game, OBJ_WALL_PANEL, vec3(-13.0f, 0.0f, -4.0f), vec3(wall_t, wall_h, 10.0f));
    add_map_object(game, OBJ_WALL_PANEL, vec3(-13.0f, 0.0f, 10.0f), vec3(wall_t, wall_h, 8.0f));

    /* Jobb szárny */
    add_map_object(game, OBJ_WALL_PANEL, vec3(13.0f, 0.0f, -4.0f), vec3(wall_t, wall_h, 10.0f));
    add_map_object(game, OBJ_WALL_PANEL, vec3(13.0f, 0.0f, 10.0f), vec3(wall_t, wall_h, 8.0f));

    /* Végső hosszú folyosó */
    add_map_object(game, OBJ_WALL_PANEL, vec3(-4.0f, 0.0f, -13.0f), vec3(wall_t, wall_h, 8.0f));
    add_map_object(game, OBJ_WALL_PANEL, vec3(4.0f, 0.0f, -13.0f), vec3(wall_t, wall_h, 8.0f));

    /* Díszletek */
    add_map_object(game, OBJ_COLUMN, vec3(-4.0f, 0.0f, 2.0f), col_scale);
    add_map_object(game, OBJ_COLUMN, vec3(4.0f, 0.0f, 2.0f), col_scale);
    add_map_object(game, OBJ_COLUMN, vec3(-4.0f, 0.0f, -2.0f), col_scale);
    add_map_object(game, OBJ_COLUMN, vec3(4.0f, 0.0f, -2.0f), col_scale);

    add_map_object(game, OBJ_PEDESTAL, vec3(-11.0f, 0.0f, 4.0f), vec3(1.5f, 1.5f, 1.5f));
    add_map_object(game, OBJ_PEDESTAL, vec3(-11.0f, 0.0f, -4.0f), vec3(1.5f, 1.5f, 1.5f));
    add_map_object(game, OBJ_PEDESTAL, vec3(11.0f, 0.0f, 4.0f), vec3(1.5f, 1.5f, 1.5f));
    add_map_object(game, OBJ_PEDESTAL, vec3(11.0f, 0.0f, -4.0f), vec3(1.5f, 1.5f, 1.5f));

    add_map_object(game, OBJ_VITRINE, vec3(-5.0f, 0.0f, 12.0f), vec3(2.0f, 1.8f, 1.6f));
    add_map_object(game, OBJ_VITRINE, vec3(5.0f, 0.0f, 12.0f), vec3(2.0f, 1.8f, 1.6f));
    add_map_object(game, OBJ_VITRINE, vec3(0.0f, 0.0f, -4.5f), vec3(2.0f, 1.8f, 1.6f));

    /* Lámpák úgy vannak rakva, hogy mindig látszódjon a következő */
    add_light(game, vec3(0.0f, 1.6f, 12.0f), 2.0f, 3.5f);
    add_light(game, vec3(-6.5f, 1.6f, 9.0f), 2.0f, 3.5f);
    add_light(game, vec3(-11.5f, 1.8f, 4.0f), 2.0f, 3.5f);
    add_light(game, vec3(-11.5f, 1.8f, -3.5f), 2.0f, 3.5f);
    add_light(game, vec3(-4.5f, 1.6f, -8.5f), 2.0f, 3.5f);
    add_light(game, vec3(0.0f, 1.6f, -13.5f), 2.0f, 3.5f);
    add_light(game, vec3(4.5f, 1.6f, -8.5f), 2.0f, 3.5f);
    add_light(game, vec3(11.5f, 1.8f, -3.5f), 2.0f, 3.5f);
    add_light(game, vec3(11.5f, 1.8f, 4.0f), 2.0f, 3.5f);
    add_light(game, vec3(6.5f, 1.6f, 9.0f), 2.0f, 3.5f);

    set_active_light(game, 0);
}

void init_game(GameState* game)
{
    reset_game(game);
}

void reset_game(GameState* game)
{
    int i;

    game->player.position = vec3(0.0f, PLAYER_HEIGHT, 14.0f);
    game->player.yaw = 0.0f;
    game->player.pitch = 0.0f;
    game->player.radius = 0.35f;
    game->player.move_speed = 3.5f;
    game->player.y_velocity = 0.0f;
    game->player.is_jumping = 0;

    game->max_time = 5.5f;
    game->time_remaining = game->max_time;

    game->game_over = 0;
    game->win_counter = 0;

    build_level(game);

    for (i = 0; i < game->light_point_count; i++) {
        game->light_points[i].collected = 0;
        game->light_points[i].current_intensity = 0.25f;
    }

    /* Kezdő biztonsági lámpa már ég */
    if (game->light_point_count > 0) {
        game->light_points[0].collected = 1;
        game->light_points[0].current_intensity = 1.2f;
    }

    if (game->light_point_count > 1) {
        set_active_light(game, 1);
    }
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
    Vec3 step;
    Vec3 try_x;
    Vec3 try_z;

    if (key_state[SDL_SCANCODE_LSHIFT] || key_state[SDL_SCANCODE_RSHIFT]) {
        speed = 5.4f;
    }

    if (key_state[SDL_SCANCODE_W]) move_dir = vec3_add(move_dir, forward);
    if (key_state[SDL_SCANCODE_S]) move_dir = vec3_sub(move_dir, forward);
    if (key_state[SDL_SCANCODE_A]) move_dir = vec3_sub(move_dir, right);
    if (key_state[SDL_SCANCODE_D]) move_dir = vec3_add(move_dir, right);

    if (vec3_length(move_dir) > 0.0001f) {
        step = vec3_scale(vec3_normalize(move_dir), speed * dt);

        try_x = game->player.position;
        try_x.x += step.x;
        if (can_move_to(game, try_x)) {
            game->player.position.x = try_x.x;
        }

        try_z = game->player.position;
        try_z.z += step.z;
        if (can_move_to(game, try_z)) {
            game->player.position.z = try_z.z;
        }
    }

    if (key_state[SDL_SCANCODE_SPACE] && !game->player.is_jumping) {
        game->player.y_velocity = JUMP_FORCE;
        game->player.is_jumping = 1;
    }

    game->player.y_velocity += GRAVITY * dt;
    game->player.position.y += game->player.y_velocity * dt;

    if (game->player.position.y <= PLAYER_HEIGHT) {
        game->player.position.y = PLAYER_HEIGHT;
        game->player.y_velocity = 0.0f;
        game->player.is_jumping = 0;
    }
}

static int is_inside_any_collected_light(const GameState* game)
{
    int i;

    for (i = 0; i < game->light_point_count; i++) {
        if (game->light_points[i].collected) {
            if (distance_xz(game->player.position, game->light_points[i].position) <= game->light_points[i].safe_radius) {
                return 1;
            }
        }
    }

    return 0;
}

static void update_light_interaction(GameState* game, float dt, const unsigned char* key_state)
{
    LightPoint* target;
    float dist;

    if (game->current_target < 0 || game->current_target >= game->light_point_count) {
        return;
    }

    target = &game->light_points[game->current_target];
    dist = distance_xz(game->player.position, target->position);

    /* Csak a közelben lévő, kinézett cél-lámpát lehessen állítani */
    if (target->active && dist <= target->reach_radius && is_looking_at_light(game, target)) {
        if (key_state[SDL_SCANCODE_EQUALS] || key_state[SDL_SCANCODE_KP_PLUS]) {
            target->current_intensity += 1.4f * dt;
        }

        if (key_state[SDL_SCANCODE_MINUS] || key_state[SDL_SCANCODE_KP_MINUS]) {
            target->current_intensity -= 1.4f * dt;
        }

        if (target->current_intensity < 0.15f) {
            target->current_intensity = 0.15f;
        }

        if (target->current_intensity > 1.8f) {
            target->current_intensity = 1.8f;
        }

        /* Ha elég erős, akkor végleg bekapcsolt lámpa lesz */
        if (target->current_intensity >= 0.9f) {
            target->active = 0;
            target->collected = 1;
            game->win_counter++;
            game->time_remaining = game->max_time;

            if (game->current_target + 1 < game->light_point_count) {
                set_active_light(game, game->current_target + 1);
            }
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
    update_light_interaction(game, dt, key_state);

    if (!is_inside_any_collected_light(game)) {
        game->time_remaining -= dt;
        if (game->time_remaining <= 0.0f) {
            game->time_remaining = 0.0f;
            game->game_over = 1;
        }
    } else {
        if (game->time_remaining > game->max_time) {
            game->time_remaining = game->max_time;
        }
    }
}