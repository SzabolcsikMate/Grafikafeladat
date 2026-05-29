#ifndef RENDER_GEOMETRY_H
#define RENDER_GEOMETRY_H

#include <GL/gl.h>
#include "math3d.h"

void draw_box(Vec3 min, Vec3 max, float r, float g, float b);
void draw_textured_box(Vec3 min, Vec3 max, GLuint texture_id, float tex_scale);
void draw_textured_box_tinted(Vec3 min, Vec3 max, GLuint texture_id, float tex_scale, float tr, float tg, float tb);

#endif
