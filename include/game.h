#ifndef GAME_H
#define GAME_H

#include "math3d.h"
#include "collision.h"

#define MAX_COLLIDERS 512
#define MAX_LIGHT_POINTS 24
#define MAX_MAP_OBJECTS 256
#define PLAYER_HEIGHT 1.0f

typedef enum {
    OBJ_COLUMN,
    OBJ_PEDESTAL,
    OBJ_WALL_PANEL,
    OBJ_BOUNDING_WALL,
    OBJ_PLATFORM,
    OBJ_LOW_BLOCK
} ObjectType;

typedef struct {
    ObjectType type;
    Vec3 position;
    Vec3 scale;
} MapObject;

typedef struct Player {
    Vec3 position;
    float yaw;
    float pitch;
    float radius;
    float move_speed;
    float y_velocity;
    int is_jumping;
} Player;

typedef struct LightPoint {
    Vec3 position;
    float current_intensity;
    int active;
    int collected;
    float reach_radius;
    float safe_radius;
} LightPoint;

typedef struct GameState {
    Player player;

    AABB colliders[MAX_COLLIDERS];
    int collider_count;

    MapObject map_objects[MAX_MAP_OBJECTS];
    int object_count;

    LightPoint light_points[MAX_LIGHT_POINTS];
    int light_point_count;
    int current_target;

    float time_remaining;
    float max_time;

    float game_over_fade;
    int game_over;
    int dying;
    float darkness_alpha;
    int win_counter;

    int selected_light_index;
    float selected_light_ratio;

    int inverted_mouse;

    int escaped;
    int end_screen;

    Vec3 exit_position;
    float exit_radius;

    float time_left;
} GameState;

void init_game(GameState* game);
void reset_game(GameState* game);
void update_game(GameState* game, float dt, const unsigned char* key_state, int mouse_dx, int mouse_dy, int* quit_requested);
void toggle_help(void);

#endif