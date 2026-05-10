#include <math.h>
#include <SDL2/SDL.h>
#include "game.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static const float MOUSE_SENSITIVITY = 0.08f;
static const float GRAVITY = -16.0f;
static const float JUMP_FORCE = 6.8f;
static const float PLAYER_HEIGHT = 1.0f;
static const float INTERACT_LOOK_DOT = 0.55f;
static const float LAMP_MIN_POWER = 0.08f;
static const float LAMP_MAX_POWER = 1.20f;

static void set_active_light(GameState* game, int index);
static int can_move_to(GameState* game, Vec3 new_pos);
static float distance_xz(Vec3 a, Vec3 b);
static int is_looking_at_light(const GameState* game, const LightPoint* light);
static Vec3 get_forward_3d(const GameState* game);
static void add_corner_fill(GameState* game, float x, float z);

static void add_collider(GameState* game, Vec3 min, Vec3 max)
{
    if (game->collider_count >= MAX_COLLIDERS) {
        return;
    }

    game->colliders[game->collider_count].min = min;
    game->colliders[game->collider_count].max = max;
    game->collider_count++;
}

static void add_floor_collider(GameState* game, Vec3 center, Vec3 scale)
{
    add_collider(
        game,
        vec3(center.x - scale.x * 0.5f, -0.1f, center.z - scale.z * 0.5f),
        vec3(center.x + scale.x * 0.5f, 0.0f, center.z + scale.z * 0.5f)
    );
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
    game->light_points[game->light_point_count].current_intensity = LAMP_MIN_POWER;
    game->light_points[game->light_point_count].active = 0;
    game->light_points[game->light_point_count].collected = 0;
    game->light_points[game->light_point_count].reach_radius = reach;
    game->light_points[game->light_point_count].safe_radius = safe_radius;
    game->light_point_count++;
}

static void add_floor_object(GameState* game, Vec3 center, Vec3 scale)
{
    add_floor_collider(game, center, scale);
}

static void build_room(GameState* game, Vec3 center, float sx, float sz, int open_north, int open_south, int open_east, int open_west)
{
    float wall_h = 4.6f;
    float wall_t = 0.7f;
    float half_x = sx * 0.5f;
    float half_z = sz * 0.5f;
    float door_w = 3.2f;

    add_floor_object(game, center, vec3(sx, 0.1f, sz));

    if (!open_north) {
        add_map_object(game, OBJ_BOUNDING_WALL, vec3(center.x, 0.0f, center.z - half_z), vec3(sx, wall_h, wall_t));
    } else {
        add_map_object(game, OBJ_BOUNDING_WALL, vec3(center.x - (half_x + door_w * 0.5f) * 0.5f, 0.0f, center.z - half_z), vec3(half_x - door_w * 0.5f, wall_h, wall_t));
        add_map_object(game, OBJ_BOUNDING_WALL, vec3(center.x + (half_x + door_w * 0.5f) * 0.5f, 0.0f, center.z - half_z), vec3(half_x - door_w * 0.5f, wall_h, wall_t));
    }

    if (!open_south) {
        add_map_object(game, OBJ_BOUNDING_WALL, vec3(center.x, 0.0f, center.z + half_z), vec3(sx, wall_h, wall_t));
    } else {
        add_map_object(game, OBJ_BOUNDING_WALL, vec3(center.x - (half_x + door_w * 0.5f) * 0.5f, 0.0f, center.z + half_z), vec3(half_x - door_w * 0.5f, wall_h, wall_t));
        add_map_object(game, OBJ_BOUNDING_WALL, vec3(center.x + (half_x + door_w * 0.5f) * 0.5f, 0.0f, center.z + half_z), vec3(half_x - door_w * 0.5f, wall_h, wall_t));
    }

    if (!open_west) {
        add_map_object(game, OBJ_BOUNDING_WALL, vec3(center.x - half_x, 0.0f, center.z), vec3(wall_t, wall_h, sz));
    } else {
        add_map_object(game, OBJ_BOUNDING_WALL, vec3(center.x - half_x, 0.0f, center.z - (half_z + door_w * 0.5f) * 0.5f), vec3(wall_t, wall_h, half_z - door_w * 0.5f));
        add_map_object(game, OBJ_BOUNDING_WALL, vec3(center.x - half_x, 0.0f, center.z + (half_z + door_w * 0.5f) * 0.5f), vec3(wall_t, wall_h, half_z - door_w * 0.5f));
    }

    if (!open_east) {
        add_map_object(game, OBJ_BOUNDING_WALL, vec3(center.x + half_x, 0.0f, center.z), vec3(wall_t, wall_h, sz));
    } else {
        add_map_object(game, OBJ_BOUNDING_WALL, vec3(center.x + half_x, 0.0f, center.z - (half_z + door_w * 0.5f) * 0.5f), vec3(wall_t, wall_h, half_z - door_w * 0.5f));
        add_map_object(game, OBJ_BOUNDING_WALL, vec3(center.x + half_x, 0.0f, center.z + (half_z + door_w * 0.5f) * 0.5f), vec3(wall_t, wall_h, half_z - door_w * 0.5f));
    }
}

static void build_corridor(GameState* game, Vec3 center, float sx, float sz, int east_west)
{
    float wall_h = 4.6f;
    float wall_t = 0.7f;

    add_floor_object(game, center, vec3(sx, 0.1f, sz));

    if (east_west) {
        add_map_object(game, OBJ_BOUNDING_WALL, vec3(center.x, 0.0f, center.z - sz * 0.5f), vec3(sx, wall_h, wall_t));
        add_map_object(game, OBJ_BOUNDING_WALL, vec3(center.x, 0.0f, center.z + sz * 0.5f), vec3(sx, wall_h, wall_t));
    } else {
        add_map_object(game, OBJ_BOUNDING_WALL, vec3(center.x - sx * 0.5f, 0.0f, center.z), vec3(wall_t, wall_h, sz));
        add_map_object(game, OBJ_BOUNDING_WALL, vec3(center.x + sx * 0.5f, 0.0f, center.z), vec3(wall_t, wall_h, sz));
    }
}

static void build_parkour(GameState* game, Vec3 center)
{
    float wall_h = 4.6f;
    float wall_t = 0.7f;

    add_map_object(game, OBJ_BOUNDING_WALL, vec3(center.x - 3.6f, 0.0f, center.z), vec3(wall_t, wall_h, 28.0f));
    add_map_object(game, OBJ_BOUNDING_WALL, vec3(center.x + 3.6f, 0.0f, center.z), vec3(wall_t, wall_h, 28.0f));

    /* parkour bejárat, erre még biztonságosan rá tudsz lépni */
    add_map_object(game, OBJ_PLATFORM, vec3(center.x, 0.0f, center.z + 13.0f), vec3(5.8f, 0.35f, 4.0f));

    /* jelölő kapu */
    add_map_object(game, OBJ_LOW_BLOCK, vec3(center.x, 0.0f, center.z + 15.2f), vec3(5.8f, 0.20f, 0.35f));
    add_map_object(game, OBJ_LOW_BLOCK, vec3(center.x - 2.8f, 0.0f, center.z + 13.0f), vec3(0.35f, 0.20f, 4.0f));
    add_map_object(game, OBJ_LOW_BLOCK, vec3(center.x + 2.8f, 0.0f, center.z + 13.0f), vec3(0.35f, 0.20f, 4.0f));

    /* csak két ugrás */
    add_map_object(game, OBJ_PLATFORM, vec3(center.x - 1.4f, 0.0f, center.z + 6.5f), vec3(3.2f, 0.75f, 3.2f));
    add_map_object(game, OBJ_PLATFORM, vec3(center.x + 1.4f, 0.0f, center.z), vec3(3.2f, 1.05f, 3.2f));

    /* kijáró platform */
    add_map_object(game, OBJ_PLATFORM, vec3(center.x, 0.0f, center.z - 7.0f), vec3(5.4f, 0.45f, 4.0f));
}

static void add_floor_piece(GameState* game, Vec3 pos, Vec3 scale)
{
    if (game->object_count >= MAX_MAP_OBJECTS) {
        return;
    }

    game->map_objects[game->object_count].type = OBJ_PLATFORM;
    game->map_objects[game->object_count].position = pos;
    game->map_objects[game->object_count].scale = scale;
    game->object_count++;

    add_floor_collider(game, pos, vec3(scale.x, 0.1f, scale.z));
}

static void add_ceiling_piece(GameState* game, Vec3 pos, Vec3 scale)
{
    if (game->object_count >= MAX_MAP_OBJECTS) {
        return;
    }

    game->map_objects[game->object_count].type = OBJ_LOW_BLOCK;
    game->map_objects[game->object_count].position = pos;
    game->map_objects[game->object_count].scale = scale;
    game->object_count++;
}

static void add_corridor_z(GameState* game, float x, float z, float length)
{
    float wall_h = 4.6f;
    float wall_t = 0.7f;

    add_floor_piece(game, vec3(x, 0.0f, z), vec3(7.0f, 0.08f, length));
    add_ceiling_piece(game, vec3(x, 4.6f, z), vec3(7.0f, 0.08f, length));

    add_map_object(game, OBJ_BOUNDING_WALL, vec3(x - 3.5f, 0.0f, z), vec3(wall_t, wall_h, length));
    add_map_object(game, OBJ_BOUNDING_WALL, vec3(x + 3.5f, 0.0f, z), vec3(wall_t, wall_h, length));
}

static void add_corridor_x(GameState* game, float x, float z, float length)
{
    float wall_h = 4.6f;
    float wall_t = 0.7f;

    add_floor_piece(game, vec3(x, 0.0f, z), vec3(length, 0.08f, 7.0f));
    add_ceiling_piece(game, vec3(x, 4.6f, z), vec3(length, 0.08f, 7.0f));

    add_map_object(game, OBJ_BOUNDING_WALL, vec3(x, 0.0f, z - 3.5f), vec3(length, wall_h, wall_t));
    add_map_object(game, OBJ_BOUNDING_WALL, vec3(x, 0.0f, z + 3.5f), vec3(length, wall_h, wall_t));
}

static void add_corner_fill(GameState* game, float x, float z)
{
    add_floor_piece(game, vec3(x, 0.0f, z), vec3(7.0f, 0.08f, 7.0f));
    add_ceiling_piece(game, vec3(x, 4.6f, z), vec3(7.0f, 0.08f, 7.0f));
}

static void add_corner_wall_block(GameState* game, float x, float z)
{
    add_map_object(
        game,
        OBJ_BOUNDING_WALL,
        vec3(x, 0.0f, z),
        vec3(7.0f, 3.2f, 0.7f)
    );
}

static void add_turn_room(GameState* game, float x, float z)
{
    add_floor_piece(game, vec3(x, 0.0f, z), vec3(9.0f, 0.08f, 9.0f));
    add_ceiling_piece(game, vec3(x, 4.6f, z), vec3(9.0f, 0.08f, 9.0f));
}


static void add_room_wall_z(GameState* game, float x, float z, float sx, int open_center)
{
    float wall_h = 4.6f;
    float wall_t = 0.7f;
    float door_w = 7.0f;

    if (!open_center) {
        add_map_object(game, OBJ_BOUNDING_WALL, vec3(x, 0.0f, z), vec3(sx, wall_h, wall_t));
        return;
    }

    add_map_object(game, OBJ_BOUNDING_WALL, vec3(x - (sx + door_w) * 0.25f, 0.0f, z), vec3((sx - door_w) * 0.5f, wall_h, wall_t));
    add_map_object(game, OBJ_BOUNDING_WALL, vec3(x + (sx + door_w) * 0.25f, 0.0f, z), vec3((sx - door_w) * 0.5f, wall_h, wall_t));
}

static void add_room_wall_x(GameState* game, float x, float z, float sz, int open_center)
{
    float wall_h = 4.6f;
    float wall_t = 0.7f;
    float door_w = 7.0f;

    if (!open_center) {
        add_map_object(game, OBJ_BOUNDING_WALL, vec3(x, 0.0f, z), vec3(wall_t, wall_h, sz));
        return;
    }

    add_map_object(game, OBJ_BOUNDING_WALL, vec3(x, 0.0f, z - (sz + door_w) * 0.25f), vec3(wall_t, wall_h, (sz - door_w) * 0.5f));
    add_map_object(game, OBJ_BOUNDING_WALL, vec3(x, 0.0f, z + (sz + door_w) * 0.25f), vec3(wall_t, wall_h, (sz - door_w) * 0.5f));
}

static void add_node_room(GameState* game, float x, float z, int open_north, int open_south, int open_east, int open_west)
{
    float size = 11.0f;
    float half = size * 0.5f;

    add_floor_piece(game, vec3(x, 0.0f, z), vec3(size, 0.08f, size));
    add_ceiling_piece(game, vec3(x, 4.6f, z), vec3(size, 0.08f, size));

    add_room_wall_z(game, x, z - half, size, open_north);
    add_room_wall_z(game, x, z + half, size, open_south);
    add_room_wall_x(game, x + half, z, size, open_east);
    add_room_wall_x(game, x - half, z, size, open_west);
}

static void add_corridor_between_z(GameState* game, float x, float z1, float z2)
{
    float room = 11.0f;
    float wall_h = 4.6f;
    float wall_t = 0.7f;
    float min_z;
    float max_z;
    float len;
    float center_z;

    if (z1 < z2) {
        min_z = z1 + room * 0.5f;
        max_z = z2 - room * 0.5f;
    } else {
        min_z = z2 + room * 0.5f;
        max_z = z1 - room * 0.5f;
    }

    len = max_z - min_z;
    if (len <= 0.1f) {
        return;
    }

    center_z = (min_z + max_z) * 0.5f;

    add_floor_piece(game, vec3(x, 0.0f, center_z), vec3(7.0f, 0.08f, len));
    add_ceiling_piece(game, vec3(x, 4.6f, center_z), vec3(7.0f, 0.08f, len));

    add_map_object(game, OBJ_BOUNDING_WALL, vec3(x - 3.5f, 0.0f, center_z), vec3(wall_t, wall_h, len));
    add_map_object(game, OBJ_BOUNDING_WALL, vec3(x + 3.5f, 0.0f, center_z), vec3(wall_t, wall_h, len));
}

static void add_corridor_between_x(GameState* game, float x1, float x2, float z)
{
    float room = 11.0f;
    float wall_h = 4.6f;
    float wall_t = 0.7f;
    float min_x;
    float max_x;
    float len;
    float center_x;

    if (x1 < x2) {
        min_x = x1 + room * 0.5f;
        max_x = x2 - room * 0.5f;
    } else {
        min_x = x2 + room * 0.5f;
        max_x = x1 - room * 0.5f;
    }

    len = max_x - min_x;
    if (len <= 0.1f) {
        return;
    }

    center_x = (min_x + max_x) * 0.5f;

    add_floor_piece(game, vec3(center_x, 0.0f, z), vec3(len, 0.08f, 7.0f));
    add_ceiling_piece(game, vec3(center_x, 4.6f, z), vec3(len, 0.08f, 7.0f));

    add_map_object(game, OBJ_BOUNDING_WALL, vec3(center_x, 0.0f, z - 3.5f), vec3(len, wall_h, wall_t));
    add_map_object(game, OBJ_BOUNDING_WALL, vec3(center_x, 0.0f, z + 3.5f), vec3(len, wall_h, wall_t));
}

static void add_parkour_between_z(GameState* game, float x, float z1, float z2)
{
    float room = 11.0f;
    float wall_h = 4.6f;
    float wall_t = 0.7f;
    float min_z;
    float max_z;
    float center_z;

    if (z1 < z2) {
        min_z = z1 + room * 0.5f;
        max_z = z2 - room * 0.5f;
    } else {
        min_z = z2 + room * 0.5f;
        max_z = z1 - room * 0.5f;
    }

    center_z = (min_z + max_z) * 0.5f;

    add_ceiling_piece(game, vec3(x, 4.6f, center_z), vec3(7.0f, 0.08f, max_z - min_z));
    add_map_object(game, OBJ_BOUNDING_WALL, vec3(x - 3.5f, 0.0f, center_z), vec3(wall_t, wall_h, max_z - min_z));
    add_map_object(game, OBJ_BOUNDING_WALL, vec3(x + 3.5f, 0.0f, center_z), vec3(wall_t, wall_h, max_z - min_z));

    add_map_object(game, OBJ_PLATFORM, vec3(x, 0.0f, max_z - 2.0f), vec3(5.4f, 0.35f, 3.0f));
    add_map_object(game, OBJ_PLATFORM, vec3(x - 1.3f, 0.0f, center_z + 2.5f), vec3(3.1f, 0.70f, 3.1f));
    add_map_object(game, OBJ_PLATFORM, vec3(x + 1.3f, 0.0f, center_z - 2.5f), vec3(3.1f, 1.00f, 3.1f));
    add_map_object(game, OBJ_PLATFORM, vec3(x, 0.0f, min_z + 2.0f), vec3(5.4f, 0.45f, 3.0f));
}

static void add_parkour_between_x(GameState* game, float x1, float x2, float z)
{
    float room = 11.0f;
    float wall_h = 4.6f;
    float wall_t = 0.7f;
    float min_x;
    float max_x;
    float center_x;

    if (x1 < x2) {
        min_x = x1 + room * 0.5f;
        max_x = x2 - room * 0.5f;
    } else {
        min_x = x2 + room * 0.5f;
        max_x = x1 - room * 0.5f;
    }

    center_x = (min_x + max_x) * 0.5f;

    add_ceiling_piece(game, vec3(center_x, 4.6f, z), vec3(max_x - min_x, 0.08f, 7.0f));
    add_map_object(game, OBJ_BOUNDING_WALL, vec3(center_x, 0.0f, z - 3.5f), vec3(max_x - min_x, wall_h, wall_t));
    add_map_object(game, OBJ_BOUNDING_WALL, vec3(center_x, 0.0f, z + 3.5f), vec3(max_x - min_x, wall_h, wall_t));

    add_map_object(game, OBJ_PLATFORM, vec3(min_x + 2.0f, 0.0f, z), vec3(3.0f, 0.35f, 5.4f));
    add_map_object(game, OBJ_PLATFORM, vec3(center_x - 2.5f, 0.0f, z - 1.3f), vec3(3.1f, 0.70f, 3.1f));
    add_map_object(game, OBJ_PLATFORM, vec3(center_x + 2.5f, 0.0f, z + 1.3f), vec3(3.1f, 1.00f, 3.1f));
    add_map_object(game, OBJ_PLATFORM, vec3(max_x - 2.0f, 0.0f, z), vec3(3.0f, 0.45f, 5.4f));
}

static void build_level(GameState* game)
{
    game->collider_count = 0;
    game->object_count = 0;
    game->light_point_count = 0;

    add_node_room(game, 0.0f, 18.0f, 1, 0, 0, 0);
    add_node_room(game, 0.0f, 0.0f, 0, 1, 0, 1);
    add_node_room(game, -18.0f, 0.0f, 1, 0, 1, 0);
    add_node_room(game, -18.0f, -22.0f, 0, 1, 0, 1);
    add_node_room(game, -38.0f, -22.0f, 1, 0, 1, 0);
    add_node_room(game, -38.0f, -48.0f, 0, 1, 1, 0);
    add_node_room(game, -14.0f, -48.0f, 1, 0, 0, 1);
    add_node_room(game, -14.0f, -74.0f, 0, 1, 1, 0);
    add_node_room(game, 10.0f, -74.0f, 1, 0, 0, 1);
    add_node_room(game, 10.0f, -104.0f, 0, 1, 1, 0);
    add_node_room(game, 34.0f, -104.0f, 0, 0, 1, 1);

    add_corridor_between_z(game, 0.0f, 18.0f, 0.0f);
    add_corridor_between_x(game, 0.0f, -18.0f, 0.0f);
    add_corridor_between_z(game, -18.0f, 0.0f, -22.0f);
    add_parkour_between_x(game, -18.0f, -38.0f, -22.0f);
    add_parkour_between_z(game, -38.0f, -22.0f, -48.0f);
    add_corridor_between_x(game, -38.0f, -14.0f, -48.0f);
    add_parkour_between_z(game, -14.0f, -48.0f, -74.0f);
    add_corridor_between_x(game, -14.0f, 10.0f, -74.0f);
    add_corridor_between_z(game, 10.0f, -74.0f, -104.0f);
    add_corridor_between_x(game, 10.0f, 34.0f, -104.0f);

    add_light(game, vec3(0.0f, 1.8f, 18.0f), 4.4f, 5.8f);
    add_light(game, vec3(0.0f, 1.8f, 0.0f), 4.4f, 5.8f);
    add_light(game, vec3(-18.0f, 1.8f, 0.0f), 4.4f, 5.8f);
    add_light(game, vec3(-18.0f, 1.8f, -22.0f), 4.4f, 5.8f);
    add_light(game, vec3(-38.0f, 1.8f, -22.0f), 4.4f, 5.8f);
    add_light(game, vec3(-38.0f, 1.8f, -48.0f), 4.4f, 5.8f);
    add_light(game, vec3(-14.0f, 1.8f, -48.0f), 4.4f, 5.8f);
    add_light(game, vec3(-14.0f, 1.8f, -74.0f), 4.4f, 5.8f);
    add_light(game, vec3(10.0f, 1.8f, -74.0f), 4.4f, 5.8f);
    add_light(game, vec3(10.0f, 1.8f, -104.0f), 4.4f, 5.8f);

    game->exit_position = vec3(39.8f, 1.0f, -104.0f);
    game->exit_radius = 3.4f;

    add_floor_piece(game, vec3(42.0f, 0.0f, -104.0f), vec3(5.0f, 0.08f, 7.0f));
    add_ceiling_piece(game, vec3(42.0f, 4.6f, -104.0f), vec3(5.0f, 0.08f, 7.0f));

    set_active_light(game, 0);
}

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

static int is_inside_any_safe_light(const GameState* game)
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

static void update_light_interaction(GameState* game, float dt, const unsigned char* key_state)
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
static void refresh_time_in_lit_lamp(GameState* game)
{
    int i;

    for (i = 0; i < game->light_point_count; i++) {
        if (game->light_points[i].collected) {
            if (distance_xz(game->player.position, game->light_points[i].position) <= game->light_points[i].safe_radius) {
                game->time_remaining = game->max_time;
                return;
            }
        }
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

    handle_mouse_look(game, mouse_dx, mouse_dy);
    handle_movement(game, dt, key_state);
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

static void set_active_light(GameState* game, int index)
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