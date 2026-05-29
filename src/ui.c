#include <math.h>
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include "ui.h"
#include "font.h"
#include "game.h"

static void draw_ui_rect_px(float x1, float y1, float x2, float y2, float r, float g, float b, float a);
static void draw_ui_outline_px(float x1, float y1, float x2, float y2, float r, float g, float b, float a);
static void draw_lamp_power_slider(const GameState* game);
static void draw_countdown(const GameState* game);
static void draw_end_screen(const GameState* game);

static void draw_lamp_power_slider(const GameState* game)
{
    float ratio;

    if (game->selected_light_index < 0) {
        return;
    }

    ratio = game->selected_light_ratio;

    if (ratio < 0.0f) {
        ratio = 0.0f;
    }

    if (ratio > 1.0f) {
        ratio = 1.0f;
    }

    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.0f, 0.0f, 0.0f, 0.78f);
    glBegin(GL_QUADS);
    glVertex2f(0.32f, 0.18f);
    glVertex2f(0.68f, 0.18f);
    glVertex2f(0.68f, 0.225f);
    glVertex2f(0.32f, 0.225f);
    glEnd();

    glColor4f(1.0f, 0.78f, 0.05f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(0.33f, 0.190f);
    glVertex2f(0.33f + 0.34f * ratio, 0.190f);
    glVertex2f(0.33f + 0.34f * ratio, 0.215f);
    glVertex2f(0.33f, 0.215f);
    glEnd();

    glDisable(GL_BLEND);

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FOG);
    glEnable(GL_LIGHTING);
}

static void draw_digit_segment(float x, float y, float w, float h)
{
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

static void draw_digit(float x, float y, float s, int digit)
{
    int seg[10][7] = {
        {1,1,1,1,1,1,0},
        {0,1,1,0,0,0,0},
        {1,1,0,1,1,0,1},
        {1,1,1,1,0,0,1},
        {0,1,1,0,0,1,1},
        {1,0,1,1,0,1,1},
        {1,0,1,1,1,1,1},
        {1,1,1,0,0,0,0},
        {1,1,1,1,1,1,1},
        {1,1,1,1,0,1,1}
    };

    float t = s * 0.16f;
    float w = s;
    float h = s * 1.8f;

    if (digit < 0 || digit > 9) {
        return;
    }

    if (seg[digit][0]) draw_digit_segment(x + t, y + h - t, w - 2.0f * t, t);
    if (seg[digit][1]) draw_digit_segment(x + w - t, y + h * 0.5f, t, h * 0.5f - t);
    if (seg[digit][2]) draw_digit_segment(x + w - t, y + t, t, h * 0.5f - t);
    if (seg[digit][3]) draw_digit_segment(x + t, y, w - 2.0f * t, t);
    if (seg[digit][4]) draw_digit_segment(x, y + t, t, h * 0.5f - t);
    if (seg[digit][5]) draw_digit_segment(x, y + h * 0.5f, t, h * 0.5f - t);
    if (seg[digit][6]) draw_digit_segment(x + t, y + h * 0.5f - t * 0.5f, w - 2.0f * t, t);
}

static void draw_countdown(const GameState* game)
{
    int total_tenths;
    int seconds;
    int tenths;
    int tens;
    int ones;

    total_tenths = (int)ceilf(game->time_remaining * 10.0f);

    if (total_tenths < 0) {
        total_tenths = 0;
    }

    if (total_tenths > 990) {
        total_tenths = 990;
    }

    seconds = total_tenths / 10;
    tenths = total_tenths % 10;
    tens = seconds / 10;
    ones = seconds % 10;

    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.0f, 0.0f, 0.0f, 0.65f);
    draw_digit_segment(0.425f, 0.895f, 0.150f, 0.075f);

    glColor4f(1.0f, 0.82f, 0.18f, 1.0f);

    if (seconds >= 10) {
        draw_digit(0.440f, 0.905f, 0.023f, tens);
        draw_digit(0.480f, 0.905f, 0.023f, ones);
    } else {
        draw_digit(0.462f, 0.905f, 0.023f, ones);
    }

    draw_digit_segment(0.520f, 0.907f, 0.006f, 0.006f);
    draw_digit(0.535f, 0.905f, 0.023f, tenths);

    glDisable(GL_BLEND);

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FOG);
    glEnable(GL_LIGHTING);
}

static void draw_block_text_rect(float x, float y, float w, float h)
{
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

static void draw_simple_game_over_text(void)
{
    glColor4f(0.75f, 0.05f, 0.05f, 0.95f);

    draw_block_text_rect(0.28f, 0.53f, 0.44f, 0.012f);
    draw_block_text_rect(0.28f, 0.48f, 0.44f, 0.012f);
    draw_block_text_rect(0.28f, 0.43f, 0.44f, 0.012f);

    glColor4f(0.95f, 0.95f, 0.95f, 0.90f);

    draw_block_text_rect(0.32f, 0.505f, 0.36f, 0.010f);
    draw_block_text_rect(0.36f, 0.455f, 0.28f, 0.010f);
}


void render_game_ui(const GameState* game)
{
    if (!game->game_over && !game->escaped && !game->dying) {
        draw_countdown(game);
        draw_lamp_power_slider(game);
    }

    if (game->darkness_alpha > 0.0f && !game->game_over && !game->escaped) {
        glDisable(GL_LIGHTING);
        glDisable(GL_FOG);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glColor4f(0.0f, 0.0f, 0.0f, game->darkness_alpha);

        glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(1.0f, 0.0f);
        glVertex2f(1.0f, 1.0f);
        glVertex2f(0.0f, 1.0f);
        glEnd();

        glDisable(GL_BLEND);

        glPopMatrix();

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        glMatrixMode(GL_MODELVIEW);

        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_FOG);
        glEnable(GL_LIGHTING);
    }

    if (game->game_over || game->escaped) {
        draw_end_screen(game);
    }
}

static void draw_ui_rect_px(float x1, float y1, float x2, float y2, float r, float g, float b, float a)
{
    glColor4f(r, g, b, a);

    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

static void draw_ui_outline_px(float x1, float y1, float x2, float y2, float r, float g, float b, float a)
{
    glColor4f(r, g, b, a);
    glLineWidth(2.0f);

    glBegin(GL_LINE_LOOP);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();

    glLineWidth(1.0f);
}

static void draw_menu_background_px(int width, int height)
{
    draw_ui_rect_px(0.0f, 0.0f, (float)width, (float)height, 0.006f, 0.004f, 0.004f, 1.0f);

    draw_ui_rect_px(0.0f, 0.0f, (float)width, (float)height * 0.22f, 0.0f, 0.0f, 0.0f, 0.62f);
    draw_ui_rect_px(0.0f, (float)height * 0.78f, (float)width, (float)height, 0.04f, 0.012f, 0.003f, 0.42f);

    draw_ui_rect_px(0.0f, 0.0f, (float)width * 0.18f, (float)height, 0.0f, 0.0f, 0.0f, 0.58f);
    draw_ui_rect_px((float)width * 0.82f, 0.0f, (float)width, (float)height, 0.0f, 0.0f, 0.0f, 0.58f);

    draw_ui_rect_px((float)width * 0.05f, (float)height * 0.18f, (float)width * 0.18f, (float)height, 0.025f, 0.015f, 0.008f, 0.65f);
    draw_ui_rect_px((float)width * 0.82f, (float)height * 0.18f, (float)width * 0.95f, (float)height, 0.025f, 0.015f, 0.008f, 0.65f);
}

static void draw_gold_ornament(float x, float y)
{
    glColor4f(1.0f, 0.72f, 0.18f, 0.95f);
    glLineWidth(1.4f);

    glBegin(GL_LINES);

    glVertex2f(x - 18.0f, y);
    glVertex2f(x + 18.0f, y);

    glVertex2f(x, y - 9.0f);
    glVertex2f(x, y + 9.0f);

    glVertex2f(x - 10.0f, y - 6.0f);
    glVertex2f(x + 10.0f, y + 6.0f);

    glVertex2f(x - 10.0f, y + 6.0f);
    glVertex2f(x + 10.0f, y - 6.0f);

    glEnd();

    glLineWidth(1.0f);
}

static void draw_fancy_button_px(float x1, float y1, float x2, float y2, const char* text)
{
    float cx = (x1 + x2) * 0.5f;
    float cy = (y1 + y2) * 0.5f;

    draw_ui_rect_px(x1, y1, x2, y2, 0.0f, 0.0f, 0.0f, 0.82f);

    draw_ui_outline_px(x1, y1, x2, y2, 1.0f, 0.66f, 0.12f, 1.0f);
    draw_ui_outline_px(x1 + 5.0f, y1 + 5.0f, x2 - 5.0f, y2 - 5.0f, 0.85f, 0.46f, 0.08f, 0.95f);

    draw_ui_rect_px(x1 + 22.0f, y1 + 3.0f, x2 - 22.0f, y1 + 5.0f, 1.0f, 0.78f, 0.22f, 0.70f);
    draw_ui_rect_px(x1 + 22.0f, y2 - 5.0f, x2 - 22.0f, y2 - 3.0f, 1.0f, 0.55f, 0.08f, 0.55f);

    draw_gold_ornament(cx, y1);
    draw_gold_ornament(cx, y2);

    draw_gold_ornament(x1 + 8.0f, cy);
    draw_gold_ornament(x2 - 8.0f, cy);

    draw_centered_text_px(
        font_mid,
        text,
        cx,
        y1 + (y2 - y1) * 0.63f,
        20.0f,
        0.96f,
        0.80f,
        0.52f
    );
}

static void draw_end_screen(const GameState* game)
{
    int width = 1280;
    int height = 720;
    SDL_Window* window = SDL_GL_GetCurrentWindow();

    float cx;
    float button_w;
    float button_h;
    float title_y;
    float first_button_y;

    if (window != NULL) {
        SDL_GetWindowSize(window, &width, &height);
    }

    init_menu_fonts();

    cx = (float)width * 0.5f;
    button_w = (float)width * 0.42f;
    button_h = (float)height * 0.095f;
    title_y = (float)height * 0.28f;
    first_button_y = (float)height * 0.46f;

    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, (double)width, (double)height, 0.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    draw_ui_rect_px(
    0.0f,
    0.0f,
    (float)width,
    (float)height,
    0.0f,
    0.0f,
    0.0f,
    1.0f
);

    if (game->escaped) {
        draw_centered_text_px(
            font_big,
            "YOU ESCAPED",
            cx,
            title_y,
            43.0f,
            0.95f,
            0.78f,
            0.25f
        );
    } else {
        draw_text_px(
            font_big,
            "THE DARKNESS CAUGHT YOU",
            (float)width * 0.17f,
            title_y,
            0.92f,
            0.02f,
            0.02f
        );
    }

    draw_fancy_button_px(
        cx - button_w * 0.5f,
        first_button_y,
        cx + button_w * 0.5f,
        first_button_y + button_h,
        "RESTART"
    );

    draw_fancy_button_px(
        cx - button_w * 0.5f,
        first_button_y + (float)height * 0.14f,
        cx + button_w * 0.5f,
        first_button_y + (float)height * 0.14f + button_h,
        "RETURN TO MENU"
    );

    draw_fancy_button_px(
        cx - button_w * 0.5f,
        first_button_y + (float)height * 0.28f,
        cx + button_w * 0.5f,
        first_button_y + (float)height * 0.28f + button_h,
        "EXIT GAME"
    );

    glDisable(GL_BLEND);

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FOG);
    glEnable(GL_LIGHTING);
}

void render_menu(SDL_Window* window, int fullscreen, int inverted_mouse)
{
    int width;
    int height;
    float cx;
    float button_w;
    float button_h;

    init_menu_fonts();

    SDL_GetWindowSize(window, &width, &height);
    SDL_SetWindowTitle(window, "Dark Museum | Menu");

    cx = (float)width * 0.5f;
    button_w = (float)width * 0.32f;
    button_h = (float)height * 0.09f;

    glClearColor(0.005f, 0.004f, 0.004f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, (double)width, (double)height, 0.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    draw_menu_background_px(width, height);

    draw_centered_text_px(
        font_big,
        "DARK MUSEUM",
        cx - 50.0f,
        (float)height * 0.18f,
        42.0f,
        0.92f,
        0.82f,
        0.62f
    );

    draw_fancy_button_px(
        cx - button_w * 0.5f,
        (float)height * 0.36f,
        cx + button_w * 0.5f,
        (float)height * 0.36f + button_h,
        "PLAY"
    );

    if (fullscreen) {
        draw_fancy_button_px(
            cx - button_w * 0.5f,
            (float)height * 0.49f,
            cx + button_w * 0.5f,
            (float)height * 0.49f + button_h,
            "FULLSCREEN ON"
        );
    } else {
        draw_fancy_button_px(
            cx - button_w * 0.5f,
            (float)height * 0.49f,
            cx + button_w * 0.5f,
            (float)height * 0.49f + button_h,
            "FULLSCREEN OFF"
        );
    }

    if (inverted_mouse) {
        draw_fancy_button_px(
            cx - button_w * 0.5f,
            (float)height * 0.62f,
            cx + button_w * 0.5f,
            (float)height * 0.62f + button_h,
            "INVERTED MOUSE ON"
        );
    } else {
        draw_fancy_button_px(
            cx - button_w * 0.5f,
            (float)height * 0.62f,
            cx + button_w * 0.5f,
            (float)height * 0.62f + button_h,
            "INVERTED MOUSE OFF"
        );
    }

    draw_fancy_button_px(
        cx - button_w * 0.5f,
        (float)height * 0.75f,
        cx + button_w * 0.5f,
        (float)height * 0.75f + button_h,
        "EXIT"
    );

    glDisable(GL_BLEND);

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
}