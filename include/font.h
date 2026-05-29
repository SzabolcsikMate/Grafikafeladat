#ifndef FONT_H
#define FONT_H

#include <GL/gl.h>

extern GLuint font_big;
extern GLuint font_mid;

void init_menu_fonts(void);
void draw_text_px(GLuint font_list, const char* text, float x, float y, float r, float g, float b);
void draw_centered_text_px(GLuint font_list, const char* text, float cx, float y, float approx_char_w, float r, float g, float b);

#endif
