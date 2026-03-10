#ifndef MATH3D_H
#define MATH3D_H

typedef struct Vec3 {
    float x;
    float y;
    float z;
} Vec3;

Vec3 vec3(float x, float y, float z);
Vec3 vec3_add(Vec3 a, Vec3 b);
Vec3 vec3_sub(Vec3 a, Vec3 b);
Vec3 vec3_scale(Vec3 v, float s);
float vec3_length(Vec3 v);
Vec3 vec3_normalize(Vec3 v);

#endif