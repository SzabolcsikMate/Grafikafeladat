#include "../include/collision.h"

static float clampf(float v, float min_value, float max_value)
{
    if (v < min_value) {
        return min_value;
    }

    if (v > max_value) {
        return max_value;
    }

    return v;
}

int sphere_aabb_intersect(Vec3 center, float radius, AABB box)
{
    float closest_x = clampf(center.x, box.min.x, box.max.x);
    float closest_y = clampf(center.y, box.min.y, box.max.y);
    float closest_z = clampf(center.z, box.min.z, box.max.z);

    float dx = center.x - closest_x;
    float dy = center.y - closest_y;
    float dz = center.z - closest_z;

    float dist_sq = dx * dx + dy * dy + dz * dz;
    return dist_sq <= radius * radius;
}