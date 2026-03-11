#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/gl.h>

GLuint load_texture_bmp(const char* filename);
void bind_texture(GLuint texture_id);

#endif