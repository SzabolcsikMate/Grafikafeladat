#ifndef GAME_H
#define GAME_H

#include "math3d.h"
#include "collision.h"

#define MAX_COLLIDERS 64
#define MAX_LIGHT_POINTS 8

typedef struct Player {
    Vec3 position;
    float yaw;
    float pitch;
    float radius;
    float move_speed;
} Player;

typedef struct LightPoint {
    Vec3 position;
    float base_intensity;
    int active;
} LightPoint;

typedef struct GameState {
    Player player;
    AABB colliders[MAX_COLLIDERS];
    int collider_count;
    LightPoint light_points[MAX_LIGHT_POINTS];
    int light_point_count;
    int current_target;
    float darkness_limit;
    float darkness_timer;
    float active_light_strength;
    int game_over;
    int win_counter;
} GameState;

void init_game(GameState* game);
void reset_game(GameState* game);
void update_game(GameState* game, float dt, const unsigned char* key_state, int mouse_dx, int mouse_dy, int* quit_requested);
void toggle_help(void);

#endif