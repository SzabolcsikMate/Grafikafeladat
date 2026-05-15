#include <SDL2/SDL.h>
#include "map.h"
#include "lamps.h"

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

void build_level(GameState* game)
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
