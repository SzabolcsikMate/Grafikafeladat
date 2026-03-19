#ifndef GAME_H
#define GAME_H

#include "math3d.h"
#include "collision.h"

#define MAX_COLLIDERS 256
#define MAX_LIGHT_POINTS 16
#define MAX_MAP_OBJECTS 128

typedef enum {
    OBJ_COLUMN,
    OBJ_PEDESTAL,
    OBJ_VITRINE,
    OBJ_WALL_PANEL,
    OBJ_BOUNDING_WALL
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
    float base_intensity;
    int active;
    float reach_radius;
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