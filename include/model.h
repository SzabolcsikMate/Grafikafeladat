#ifndef MODEL_H
#define MODEL_H

#include <GL/gl.h>

/* Betölt egy GLB modellt, és egy OpenGL Display List azonosítót ad vissza */
GLuint load_glb_model(const char* filepath);

#endif
