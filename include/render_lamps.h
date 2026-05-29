#ifndef RENDER_LAMPS_H
#define RENDER_LAMPS_H

#include <GL/gl.h>
#include "game.h"

void draw_lamp_object(const LightPoint* light, float pulse, GLuint lamp_model);
void draw_light_pool(const LightPoint* light, float pulse);
void draw_lamp_head_glow(const GameState* game, const LightPoint* light, float pulse);

#endif
