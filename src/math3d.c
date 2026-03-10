#include <math.h>
#include "../include/math3d.h"

Vec3 vec3(float x, float y, float z)
{
    Vec3 v = {x, y, z};
    return v;
}

Vec3 vec3_add(Vec3 a, Vec3 b)
{
    return vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vec3 vec3_sub(Vec3 a, Vec3 b)
{
    return vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vec3 vec3_scale(Vec3 v, float s)
{
    return vec3(v.x * s, v.y * s, v.z * s);
}

float vec3_length(Vec3 v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vec3 vec3_normalize(Vec3 v)
{
    float len = vec3_length(v);

    if (len <= 0.0001f) {
        return vec3(0.0f, 0.0f, 0.0f);
    }

    return vec3(v.x / len, v.y / len, v.z / len);
}