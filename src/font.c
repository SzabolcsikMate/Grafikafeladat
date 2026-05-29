#include <string.h>
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include "font.h"
#ifdef _WIN32
#include <windows.h>
#endif

GLuint font_big = 0;
GLuint font_mid = 0;

static void init_menu_font_list(GLuint* font_list, int height, const char* name)
{
#ifdef _WIN32
    HDC hdc;
    HFONT font;
    HFONT old_font;

    if (*font_list != 0) {
        return;
    }

    hdc = wglGetCurrentDC();
    if (!hdc) {
        return;
    }

    *font_list = glGenLists(96);

    font = CreateFontA(
        height,
        0,
        0,
        0,
        FW_BOLD,
        FALSE,
        FALSE,
        FALSE,
        ANSI_CHARSET,
        OUT_TT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY,
        FF_ROMAN,
        name
    );

    old_font = (HFONT)SelectObject(hdc, font);
    wglUseFontBitmapsA(hdc, 32, 96, *font_list);
    SelectObject(hdc, old_font);
    DeleteObject(font);

#else
    (void)font_list;
    (void)height;
    (void)name;
#endif
}

void init_menu_fonts(void)
{
    init_menu_font_list(&font_big, 74, "Georgia");
    init_menu_font_list(&font_mid, 30, "Georgia");
}


void draw_text_px(GLuint font_list, const char* text, float x, float y, float r, float g, float b)
{
#ifdef _WIN32
    if (font_list == 0 || text == NULL) {
        return;
    }

    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    glListBase(font_list - 32);
    glCallLists((GLsizei)strlen(text), GL_UNSIGNED_BYTE, text);

#else
    int i;
    float char_w = 12.0f;
    float char_h = 18.0f;

    (void)font_list;

    if (text == NULL) {
        return;
    }

    glColor3f(r, g, b);

    for (i = 0; text[i] != '\0'; i++) {
        if (text[i] != ' ') {
            float px = x + (float)i * char_w;

            glBegin(GL_LINE_LOOP);
            glVertex2f(px, y - char_h);
            glVertex2f(px + char_w * 0.65f, y - char_h);
            glVertex2f(px + char_w * 0.65f, y);
            glVertex2f(px, y);
            glEnd();
        }
    }

#endif
}

void draw_centered_text_px(GLuint font_list, const char* text, float cx, float y, float approx_char_w, float r, float g, float b)
{
    float width;

    if (text == NULL) {
        return;
    }

    width = (float)strlen(text) * approx_char_w;

    draw_text_px(font_list, text, cx - width * 0.5f, y, r, g, b);
}

