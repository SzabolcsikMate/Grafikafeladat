#ifndef COLLISION_H
#define COLLISION_H

#include "math3d.h"

typedef struct AABB {
    Vec3 min;
    Vec3 max;
} AABB;

int sphere_aabb_intersect(Vec3 center, float radius, AABB box);

#endif