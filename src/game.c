#include <math.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>
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
    if (game->object_count >= MAX_MAP_OBJECTS) {
        return;
    }

    game->map_objects[game->object_count].type = type;
    game->map_objects[game->object_count].position = pos;
    game->map_objects[game->object_count].scale = scale;
    game->object_count++;

    // Add collision box based on type
    Vec3 min = vec3(pos.x - scale.x * 0.5f, 0.0f, pos.z - scale.z * 0.5f);
    Vec3 max = vec3(pos.x + scale.x * 0.5f, scale.y, pos.z + scale.z * 0.5f);
    add_collider(game, min, max);
}

static void add_light(GameState* game, Vec3 pos, float reach, float intensity)
{
    if (game->light_point_count >= MAX_LIGHT_POINTS) return;

    game->light_points[game->light_point_count].position = pos;
    game->light_points[game->light_point_count].reach_radius = reach;
    game->light_points[game->light_point_count].base_intensity = intensity;
    game->light_points[game->light_point_count].active = 0;

    game->light_point_count++;
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
        "SPACE: jump\n"
        "Mouse: look around\n"
        "Shift: faster movement\n"
        "F1: help\n"
        "+ / -: change active light strength\n"
        "R / Enter: restart after game over\n"
        "ESC: quit\n\n"
        "Reach the glowing light before your darkness timer runs out.\n"
        "If you stay too long in the dark, the game ends.",
        NULL
    );
}

static void build_level(GameState* game)
{
    game->collider_count = 0;
    game->object_count = 0;
    game->light_point_count = 0;

    float wall_h = 3.0f;
    float wall_thick = 0.5f;

    // 1. BOUNDING WALLS (20x20 outer shell)
    add_map_object(game, OBJ_BOUNDING_WALL, vec3(0, 0, -10.25f), vec3(21.0f, wall_h, wall_thick)); // North
    add_map_object(game, OBJ_BOUNDING_WALL, vec3(0, 0, 10.25f), vec3(21.0f, wall_h, wall_thick));  // South
    add_map_object(game, OBJ_BOUNDING_WALL, vec3(-10.25f, 0, 0), vec3(wall_thick, wall_h, 20.0f)); // West
    add_map_object(game, OBJ_BOUNDING_WALL, vec3(10.25f, 0, 0), vec3(wall_thick, wall_h, 20.0f));  // East

    // 2. INTERNAL WALL PANELS (Creating corridors/rooms)
    // Central cross walls leaving gaps
    add_map_object(game, OBJ_WALL_PANEL, vec3(-4.0f, 0, -5.0f), vec3(wall_thick, wall_h, 6.0f));
    add_map_object(game, OBJ_WALL_PANEL, vec3(4.0f, 0, -5.0f), vec3(wall_thick, wall_h, 6.0f));
    add_map_object(game, OBJ_WALL_PANEL, vec3(-4.0f, 0, 5.0f), vec3(wall_thick, wall_h, 6.0f));
    add_map_object(game, OBJ_WALL_PANEL, vec3(4.0f, 0, 5.0f), vec3(wall_thick, wall_h, 6.0f));

    add_map_object(game, OBJ_WALL_PANEL, vec3(-5.0f, 0, -4.0f), vec3(6.0f, wall_h, wall_thick));
    add_map_object(game, OBJ_WALL_PANEL, vec3(5.0f, 0, -4.0f), vec3(6.0f, wall_h, wall_thick));
    add_map_object(game, OBJ_WALL_PANEL, vec3(-5.0f, 0, 4.0f), vec3(6.0f, wall_h, wall_thick));
    add_map_object(game, OBJ_WALL_PANEL, vec3(5.0f, 0, 4.0f), vec3(6.0f, wall_h, wall_thick));

    // 3. COLUMNS (Corners and decorations)
    Vec3 col_scale = vec3(0.8f, wall_h, 0.8f);
    add_map_object(game, OBJ_COLUMN, vec3(-4.0f, 0, -4.0f), col_scale);
    add_map_object(game, OBJ_COLUMN, vec3(4.0f, 0, -4.0f), col_scale);
    add_map_object(game, OBJ_COLUMN, vec3(-4.0f, 0, 4.0f), col_scale);
    add_map_object(game, OBJ_COLUMN, vec3(4.0f, 0, 4.0f), col_scale);

    // 4. PEDESTALS & VITRINES & LIGHTS
    Vec3 ped_scale = vec3(1.2f, 1.2f, 1.2f);
    Vec3 vit_scale = vec3(1.4f, 2.0f, 1.4f);

    // Center starting light
    add_light(game, vec3(0.0f, 1.5f, 0.0f), 2.0f, 0.5f);

    // North Room
    add_map_object(game, OBJ_PEDESTAL, vec3(0.0f, 0, -8.0f), ped_scale);
    add_light(game, vec3(0.0f, 1.8f, -8.0f), 1.5f, 0.3f);

    // South Room
    add_map_object(game, OBJ_VITRINE, vec3(0.0f, 0, 8.0f), vit_scale);
    add_light(game, vec3(0.0f, 1.0f, 8.0f), 1.8f, 0.3f);

    // West Room
    add_map_object(game, OBJ_PEDESTAL, vec3(-8.0f, 0, 0.0f), ped_scale);
    add_light(game, vec3(-8.0f, 1.8f, 0.0f), 1.5f, 0.3f);

    // East Room
    add_map_object(game, OBJ_VITRINE, vec3(8.0f, 0, 0.0f), vit_scale);
    add_light(game, vec3(8.0f, 1.0f, 0.0f), 1.8f, 0.3f);

    // Corners
    add_map_object(game, OBJ_PEDESTAL, vec3(-8.0f, 0, -8.0f), ped_scale);
    add_light(game, vec3(-8.0f, 1.8f, -8.0f), 1.5f, 0.3f);

    add_map_object(game, OBJ_PEDESTAL, vec3(8.0f, 0, 8.0f), ped_scale);
    add_light(game, vec3(8.0f, 1.8f, 8.0f), 1.5f, 0.3f);

    // Set initial target
    set_active_light(game, 0);
}

void init_game(GameState* game)
{
    srand((unsigned int)time(NULL));
    reset_game(game);
}

void reset_game(GameState* game)
{
    game->player.position = vec3(0.0f, PLAYER_HEIGHT, 0.0f);
    game->player.yaw = 0.0f;
    game->player.pitch = 0.0f;
    game->player.radius = 0.35f;
    game->player.move_speed = 3.5f;
    game->player.y_velocity = 0.0f;
    game->player.is_jumping = 0;

    game->darkness_limit = 15.0f;
    game->darkness_timer = game->darkness_limit;
    game->active_light_strength = 2.0f;
    game->game_over = 0;
    game->win_counter = 0;

    build_level(game);
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

    // Handle horizontal movement and collisions
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

    // Handle jumping and gravity
    if (key_state[SDL_SCANCODE_SPACE] && !game->player.is_jumping) {
        game->player.y_velocity = JUMP_FORCE;
        game->player.is_jumping = 1;
    }

    // Apply gravity
    game->player.y_velocity += GRAVITY * dt;
    game->player.position.y += game->player.y_velocity * dt;

    // Floor collision
    if (game->player.position.y <= PLAYER_HEIGHT) {
        game->player.position.y = PLAYER_HEIGHT;
        game->player.y_velocity = 0.0f;
        game->player.is_jumping = 0;
    }
}

static void update_darkness(GameState* game, float dt)
{
    if (game->light_point_count == 0) return;
    LightPoint* target = &game->light_points[game->current_target];

    // Calculate distance only in XZ plane for logic, so jumping doesn't make you "miss" the target
    Vec3 p_xz = {game->player.position.x, 0.0f, game->player.position.z};
    Vec3 t_xz = {target->position.x, 0.0f, target->position.z};
    Vec3 to_target = vec3_sub(t_xz, p_xz);
    float dist = vec3_length(to_target);

    if (dist <= target->reach_radius) {
        game->darkness_timer = game->darkness_limit;
        game->win_counter++;

        // Pick a random new target that isn't the current one
        int next_target;
        do {
            next_target = rand() % game->light_point_count;
        } while (next_target == game->current_target && game->light_point_count > 1);

        set_active_light(game, next_target);
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