#include <math.h>
#include <SDL2/SDL.h>
#include "lamps.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define INTERACT_LOOK_DOT 0.55f

static int is_looking_at_light(const GameState* game, const LightPoint* light);
static Vec3 get_forward_3d(const GameState* game);

void add_light(GameState* game, Vec3 pos, float reach, float safe_radius)
{
    if (game->light_point_count >= MAX_LIGHT_POINTS) {
        return;
    }

    game->light_points[game->light_point_count].position = pos;
    game->light_points[game->light_point_count].current_intensity = LAMP_MIN_POWER;
    game->light_points[game->light_point_count].active = 0;
    game->light_points[game->light_point_count].collected = 0;
    game->light_points[game->light_point_count].reach_radius = reach;
    game->light_points[game->light_point_count].safe_radius = safe_radius;
    game->light_point_count++;
}
int is_inside_any_safe_light(const GameState* game)
{
    int i;

    for (i = 0; i < game->light_point_count; i++) {
        if (game->light_points[i].collected || game->light_points[i].active) {
            if (distance_xz(game->player.position, game->light_points[i].position) <= game->light_points[i].safe_radius) {
                return 1;
            }
        }
    }

    return 0;
}

void update_light_interaction(GameState* game, float dt, const unsigned char* key_state)
{
    int i;
    int selected_index = -1;
    float best_dist = 9999.0f;
    int increase;
    int decrease;
    LightPoint* selected;

    game->selected_light_index = -1;
    game->selected_light_ratio = 0.0f;

    for (i = 0; i < game->light_point_count; i++) {
        LightPoint* light = &game->light_points[i];
        float dist = distance_xz(game->player.position, light->position);

        if (!light->active && !light->collected) {
            continue;
        }

        if (dist > light->reach_radius) {
            continue;
        }

        if (!is_looking_at_light(game, light)) {
            continue;
        }

        if (dist < best_dist) {
            best_dist = dist;
            selected_index = i;
        }
    }

    if (selected_index < 0) {
        return;
    }

    selected = &game->light_points[selected_index];

    game->selected_light_index = selected_index;
    game->selected_light_ratio = selected->current_intensity / LAMP_MAX_POWER;

    if (game->selected_light_ratio < 0.0f) {
        game->selected_light_ratio = 0.0f;
    }

    if (game->selected_light_ratio > 1.0f) {
        game->selected_light_ratio = 1.0f;
    }

    increase =
        key_state[SDL_SCANCODE_E] ||
        key_state[SDL_SCANCODE_UP] ||
        key_state[SDL_SCANCODE_KP_PLUS];

    decrease =
        key_state[SDL_SCANCODE_Q] ||
        key_state[SDL_SCANCODE_DOWN] ||
        key_state[SDL_SCANCODE_KP_MINUS];

    if (increase) {
        selected->current_intensity += 0.70f * dt;
    }

    if (decrease) {
        selected->current_intensity -= 0.70f * dt;
    }

    if (selected->current_intensity < LAMP_MIN_POWER) {
        selected->current_intensity = LAMP_MIN_POWER;
    }

    if (selected->current_intensity > LAMP_MAX_POWER) {
        selected->current_intensity = LAMP_MAX_POWER;
    }

    game->selected_light_ratio = selected->current_intensity / LAMP_MAX_POWER;

    if (game->selected_light_ratio < 0.0f) {
        game->selected_light_ratio = 0.0f;
    }

    if (game->selected_light_ratio > 1.0f) {
        game->selected_light_ratio = 1.0f;
    }

    if (selected_index == game->current_target &&
        selected->current_intensity >= LAMP_MAX_POWER &&
        !selected->collected) {

        selected->collected = 1;
        selected->active = 0;

        game->win_counter++;
        game->time_remaining = game->max_time;

        if (game->current_target + 1 < game->light_point_count) {
            set_active_light(game, game->current_target + 1);
        }
    }
}
void set_active_light(GameState* game, int index)
{
    int i;

    for (i = 0; i < game->light_point_count; i++) {
        game->light_points[i].active = 0;
    }

    if (index >= 0 && index < game->light_point_count) {
        game->current_target = index;
        game->light_points[index].active = 1;

        if (game->light_points[index].current_intensity < LAMP_MIN_POWER) {
            game->light_points[index].current_intensity = LAMP_MIN_POWER;
        }
    }
}

float distance_xz(Vec3 a, Vec3 b)
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
    float dist = vec3_length(to_light);
    float dot;

    if (dist < 0.001f) {
        return 1;
    }

    to_light = vec3_scale(to_light, 1.0f / dist);

    dot =
        forward.x * to_light.x +
        forward.y * to_light.y +
        forward.z * to_light.z;

    return dot > INTERACT_LOOK_DOT;
}
