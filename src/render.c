#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include "render.h"
#include "game.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <string.h>

#define LAMP_MODEL_BASE_OFFSET 1.78f
#define LAMP_HEAD_LIGHT_OFFSET 0.78f

static void draw_ui_rect_px(float x1, float y1, float x2, float y2, float r, float g, float b, float a);
static void draw_ui_outline_px(float x1, float y1, float x2, float y2, float r, float g, float b, float a);
static void draw_end_screen(const GameState* game);

static void set_perspective(float fov_deg, float aspect, float near_plane, float far_plane)
{
    float top = near_plane * tanf(fov_deg * 0.5f * 3.14159265f / 180.0f);
    float bottom = -top;
    float right = top * aspect;
    float left = -right;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(left, right, bottom, top, near_plane, far_plane);
    glMatrixMode(GL_MODELVIEW);
}

static void apply_camera(const GameState* game)
{
    glRotatef(-game->player.pitch, 1.0f, 0.0f, 0.0f);
    glRotatef(-game->player.yaw, 0.0f, 1.0f, 0.0f);

    glTranslatef(
        -game->player.position.x,
        -game->player.position.y,
        -game->player.position.z
    );
}

static void draw_box(Vec3 min, Vec3 max, float r, float g, float b)
{
    glColor3f(r, g, b);

    glBegin(GL_QUADS);

    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(min.x, max.y, max.z);
    glVertex3f(max.x, max.y, max.z);
    glVertex3f(max.x, max.y, min.z);
    glVertex3f(min.x, max.y, min.z);

    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(min.x, min.y, min.z);
    glVertex3f(max.x, min.y, min.z);
    glVertex3f(max.x, min.y, max.z);
    glVertex3f(min.x, min.y, max.z);

    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(min.x, min.y, max.z);
    glVertex3f(max.x, min.y, max.z);
    glVertex3f(max.x, max.y, max.z);
    glVertex3f(min.x, max.y, max.z);

    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(max.x, min.y, min.z);
    glVertex3f(min.x, min.y, min.z);
    glVertex3f(min.x, max.y, min.z);
    glVertex3f(max.x, max.y, min.z);

    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(min.x, min.y, min.z);
    glVertex3f(min.x, min.y, max.z);
    glVertex3f(min.x, max.y, max.z);
    glVertex3f(min.x, max.y, min.z);

    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(max.x, min.y, max.z);
    glVertex3f(max.x, min.y, min.z);
    glVertex3f(max.x, max.y, min.z);
    glVertex3f(max.x, max.y, max.z);

    glEnd();
}

static void draw_textured_box(Vec3 min, Vec3 max, GLuint texture_id, float tex_scale)
{
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glColor3f(0.50f, 0.38f, 0.25f);

    glBegin(GL_QUADS);

    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(min.x, max.y, max.z);
    glTexCoord2f(tex_scale, 0.0f); glVertex3f(max.x, max.y, max.z);
    glTexCoord2f(tex_scale, tex_scale); glVertex3f(max.x, max.y, min.z);
    glTexCoord2f(0.0f, tex_scale); glVertex3f(min.x, max.y, min.z);

    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(min.x, min.y, min.z);
    glTexCoord2f(tex_scale, 0.0f); glVertex3f(max.x, min.y, min.z);
    glTexCoord2f(tex_scale, tex_scale); glVertex3f(max.x, min.y, max.z);
    glTexCoord2f(0.0f, tex_scale); glVertex3f(min.x, min.y, max.z);

    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(min.x, min.y, max.z);
    glTexCoord2f(tex_scale, 0.0f); glVertex3f(max.x, min.y, max.z);
    glTexCoord2f(tex_scale, tex_scale); glVertex3f(max.x, max.y, max.z);
    glTexCoord2f(0.0f, tex_scale); glVertex3f(min.x, max.y, max.z);

    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(max.x, min.y, min.z);
    glTexCoord2f(tex_scale, 0.0f); glVertex3f(min.x, min.y, min.z);
    glTexCoord2f(tex_scale, tex_scale); glVertex3f(min.x, max.y, min.z);
    glTexCoord2f(0.0f, tex_scale); glVertex3f(max.x, max.y, min.z);

    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(min.x, min.y, min.z);
    glTexCoord2f(tex_scale, 0.0f); glVertex3f(min.x, min.y, max.z);
    glTexCoord2f(tex_scale, tex_scale); glVertex3f(min.x, max.y, max.z);
    glTexCoord2f(0.0f, tex_scale); glVertex3f(min.x, max.y, min.z);

    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(max.x, min.y, max.z);
    glTexCoord2f(tex_scale, 0.0f); glVertex3f(max.x, min.y, min.z);
    glTexCoord2f(tex_scale, tex_scale); glVertex3f(max.x, max.y, min.z);
    glTexCoord2f(0.0f, tex_scale); glVertex3f(max.x, max.y, max.z);

    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

static void draw_textured_box_tinted(Vec3 min, Vec3 max, GLuint texture_id, float tex_scale, float tr, float tg, float tb)
{
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glColor3f(tr, tg, tb);

    glBegin(GL_QUADS);

    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(min.x, max.y, max.z);
    glTexCoord2f(tex_scale, 0.0f); glVertex3f(max.x, max.y, max.z);
    glTexCoord2f(tex_scale, tex_scale); glVertex3f(max.x, max.y, min.z);
    glTexCoord2f(0.0f, tex_scale); glVertex3f(min.x, max.y, min.z);

    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(min.x, min.y, min.z);
    glTexCoord2f(tex_scale, 0.0f); glVertex3f(max.x, min.y, min.z);
    glTexCoord2f(tex_scale, tex_scale); glVertex3f(max.x, min.y, max.z);
    glTexCoord2f(0.0f, tex_scale); glVertex3f(min.x, min.y, max.z);

    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(min.x, min.y, max.z);
    glTexCoord2f(tex_scale, 0.0f); glVertex3f(max.x, min.y, max.z);
    glTexCoord2f(tex_scale, tex_scale); glVertex3f(max.x, max.y, max.z);
    glTexCoord2f(0.0f, tex_scale); glVertex3f(min.x, max.y, max.z);

    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(max.x, min.y, min.z);
    glTexCoord2f(tex_scale, 0.0f); glVertex3f(min.x, min.y, min.z);
    glTexCoord2f(tex_scale, tex_scale); glVertex3f(min.x, max.y, min.z);
    glTexCoord2f(0.0f, tex_scale); glVertex3f(max.x, max.y, min.z);

    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(min.x, min.y, min.z);
    glTexCoord2f(tex_scale, 0.0f); glVertex3f(min.x, min.y, max.z);
    glTexCoord2f(tex_scale, tex_scale); glVertex3f(min.x, max.y, max.z);
    glTexCoord2f(0.0f, tex_scale); glVertex3f(min.x, max.y, min.z);

    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(max.x, min.y, max.z);
    glTexCoord2f(tex_scale, 0.0f); glVertex3f(max.x, min.y, min.z);
    glTexCoord2f(tex_scale, tex_scale); glVertex3f(max.x, max.y, min.z);
    glTexCoord2f(0.0f, tex_scale); glVertex3f(max.x, max.y, max.z);

    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}


static void draw_textured_floor(GLuint texture_id)
{
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glColor3f(0.55f, 0.48f, 0.38f);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);

    glTexCoord2f(0.0f, 0.0f);   glVertex3f(-18.0f, 0.0f, 18.0f);
    glTexCoord2f(12.0f, 0.0f);  glVertex3f(18.0f, 0.0f, 18.0f);
    glTexCoord2f(12.0f, 12.0f); glVertex3f(18.0f, 0.0f, -18.0f);
    glTexCoord2f(0.0f, 12.0f);  glVertex3f(-18.0f, 0.0f, -18.0f);

    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

static void draw_textured_ceiling(GLuint texture_id)
{
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glColor3f(0.70f, 0.62f, 0.50f);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, -1.0f, 0.0f);

    glTexCoord2f(0.0f, 0.0f);   glVertex3f(-18.0f, 3.0f, -18.0f);
    glTexCoord2f(12.0f, 0.0f);  glVertex3f(18.0f, 3.0f, -18.0f);
    glTexCoord2f(12.0f, 12.0f); glVertex3f(18.0f, 3.0f, 18.0f);
    glTexCoord2f(0.0f, 12.0f);  glVertex3f(-18.0f, 3.0f, 18.0f);

    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

static void draw_lamp_object(const LightPoint* light, float pulse, GLuint lamp_model)
{
    float model_scale;

    (void)pulse;

    model_scale = 0.010f;

    glPushMatrix();

    glTranslatef(light->position.x, light->position.y - LAMP_MODEL_BASE_OFFSET, light->position.z);
    glScalef(model_scale, model_scale, model_scale);

    glDisable(GL_TEXTURE_2D);

    if (lamp_model != 0) {
        GLfloat metal_ambient[]  = {0.075f, 0.070f, 0.060f, 1.0f};
        GLfloat metal_diffuse[]  = {0.30f, 0.29f, 0.25f, 1.0f};
        GLfloat metal_specular[] = {0.055f, 0.050f, 0.040f, 1.0f};
        GLfloat metal_shiny[]    = {8.0f};

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, metal_ambient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, metal_diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, metal_specular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, metal_shiny);

        glColor3f(0.30f, 0.29f, 0.25f);

        glCallList(lamp_model);
    } else {
        draw_box(
            vec3(-0.08f, 0.0f, -0.08f),
            vec3(0.08f, 1.75f, 0.08f),
            0.24f, 0.22f, 0.18f
        );

        draw_box(
            vec3(-0.28f, 1.65f, -0.28f),
            vec3(0.28f, 2.05f, 0.28f),
            0.60f, 0.46f, 0.26f
        );
    }

    glEnable(GL_TEXTURE_2D);

    glPopMatrix();
}

static void draw_light_pool(const LightPoint* light, float pulse)
{
    float radius;
    float alpha;
    float power;
    int i;

    (void)pulse;

    if (!light->active && !light->collected) {
        return;
    }

    power = light->current_intensity;

    if (power < 0.0f) {
        power = 0.0f;
    }

    if (power > 0.80f) {
        power = 0.80f;
    }

    radius = 1.0f + power * 2.0f;
    alpha = 0.035f + power * 0.11f;

    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_TRIANGLE_FAN);

    glColor4f(1.0f, 0.58f, 0.16f, alpha);
    glVertex3f(light->position.x, 0.03f, light->position.z);

    glColor4f(1.0f, 0.36f, 0.04f, 0.0f);

    for (i = 0; i <= 64; i++) {
        float a = (float)i / 64.0f * 2.0f * 3.14159265f;

        glVertex3f(
            light->position.x + cosf(a) * radius,
            0.03f,
            light->position.z + sinf(a) * radius
        );
    }

    glEnd();

    glDisable(GL_BLEND);

    glEnable(GL_CULL_FACE);
    glEnable(GL_FOG);
    glEnable(GL_LIGHTING);
}

static void draw_selected_lamp_outline(const GameState* game, GLuint lamp_model)
{
    const LightPoint* light;
    float model_scale;

    if (lamp_model == 0) {
        return;
    }

    if (game->selected_light_index < 0 ||
        game->selected_light_index >= game->light_point_count) {
        return;
    }

    light = &game->light_points[game->selected_light_index];
    model_scale = 0.0103f;

    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_POLYGON_BIT | GL_DEPTH_BUFFER_BIT | GL_CURRENT_BIT);

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_FOG);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glColor3f(1.0f, 0.82f, 0.08f);

    glPushMatrix();

    glTranslatef(light->position.x, light->position.y - 1.78f, light->position.z);
    glScalef(model_scale, model_scale, model_scale);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(1.4f);

    glCallList(lamp_model);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glLineWidth(1.0f);

    glPopMatrix();

    glDepthMask(GL_TRUE);
    glCullFace(GL_BACK);

    glPopAttrib();
}

static void draw_lamp_head_glow(const GameState* game, const LightPoint* light, float pulse)
{
    float power;
    float inner_radius;
    float outer_radius;
    float alpha;
    float head_y;
    int i;

    if (!light->active && !light->collected) {
        return;
    }

    power = light->current_intensity;

    if (power < 0.0f) power = 0.0f;
    if (power > 0.80f) power = 0.80f;

    head_y = light->position.y + LAMP_HEAD_LIGHT_OFFSET;

    inner_radius = 0.08f + power * 0.045f;
    outer_radius = 0.26f + power * 0.16f + pulse * 0.010f;
    alpha = 0.12f + power * 0.22f;

    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glPushMatrix();

    glTranslatef(light->position.x, head_y, light->position.z);

    glRotatef(game->player.yaw, 0.0f, 1.0f, 0.0f);
    glRotatef(game->player.pitch, 1.0f, 0.0f, 0.0f);

    glBegin(GL_TRIANGLE_FAN);

    glColor4f(1.0f, 0.76f, 0.26f, alpha);
    glVertex3f(0.0f, 0.0f, 0.0f);

    glColor4f(1.0f, 0.38f, 0.04f, 0.0f);

    for (i = 0; i <= 48; i++) {
        float a = (float)i / 48.0f * 2.0f * 3.14159265f;
        glVertex3f(cosf(a) * outer_radius, sinf(a) * outer_radius, 0.0f);
    }

    glEnd();

    glColor4f(1.0f, 0.86f, 0.42f, 0.90f);

    glBegin(GL_TRIANGLE_FAN);

    glVertex3f(0.0f, 0.0f, 0.0f);

    glColor4f(1.0f, 0.52f, 0.10f, 0.0f);

    for (i = 0; i <= 32; i++) {
        float a = (float)i / 32.0f * 2.0f * 3.14159265f;
        glVertex3f(cosf(a) * inner_radius, sinf(a) * inner_radius, 0.0f);
    }

    glEnd();

    glPopMatrix();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FOG);
    glEnable(GL_LIGHTING);
}

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

void init_render_state(void)
{
    GLfloat ambient[] = {0.095f, 0.075f, 0.055f, 1.0f};
    GLfloat fog_color[] = {0.018f, 0.018f, 0.025f, 1.0f};

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glShadeModel(GL_SMOOTH);

    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 3.0f);
    glFogf(GL_FOG_END, 15.0f);
    glFogfv(GL_FOG_COLOR, fog_color);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
}

void resize_viewport(int width, int height)
{
    if (height <= 0) {
        height = 1;
    }

    glViewport(0, 0, width, height);
    set_perspective(70.0f, (float)width / (float)height, 0.1f, 80.0f);
}

void render_scene(
    SDL_Window* window,
    const GameState* game,
    GLuint floor_texture,
    GLuint wall_texture,
    GLuint ceiling_texture,
    GLuint lamp_model
) {
    int i;
    Uint32 ticks;
    float pulse;
    char title[256];

    ticks = SDL_GetTicks();
    pulse = 0.5f + 0.5f * sinf((float)ticks * 0.006f);

    snprintf(
    title,
    sizeof(title),
    "Dark Museum"
);

    SDL_SetWindowTitle(window, title);

    glClearColor(0.015f, 0.015f, 0.025f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    apply_camera(game);

    {
        int light_index = 0;

        for (i = 0; i < 8; i++) {
            glDisable(GL_LIGHT0 + i);
        }

        for (i = 0; i < game->light_point_count && light_index < 8; i++) {
            const LightPoint* light = &game->light_points[i];
            float dx = light->position.x - game->player.position.x;
            float dz = light->position.z - game->player.position.z;
            float dist2 = dx * dx + dz * dz;

            if (i != game->current_target && dist2 > 650.0f) {
                continue;
            }

            if (light->active || light->collected) {
                float intensity;

                GLfloat light_pos[] = {
                    light->position.x,
                    light->position.y + LAMP_HEAD_LIGHT_OFFSET,
                    light->position.z,
                    1.0f
                };

                intensity = light->current_intensity;

                if (light->collected && intensity < 0.35f) {
                    intensity = 0.35f;
                }

                if (intensity < 0.05f) {
                    intensity = 0.05f;
                }

                if (intensity > 1.35f) {
                    intensity = 1.35f;
                }

                {
                    GLfloat light_diffuse[] = {
                        intensity * 1.35f,
                        intensity * 0.92f,
                        intensity * 0.42f,
                        1.0f
                    };

                    GLfloat light_specular[] = {
                        intensity * 0.015f,
                        intensity * 0.012f,
                        intensity * 0.008f,
                        1.0f
                    };

                    glEnable(GL_LIGHT0 + light_index);
                    glLightfv(GL_LIGHT0 + light_index, GL_POSITION, light_pos);
                    glLightfv(GL_LIGHT0 + light_index, GL_DIFFUSE, light_diffuse);
                    glLightfv(GL_LIGHT0 + light_index, GL_SPECULAR, light_specular);

                    glLightf(GL_LIGHT0 + light_index, GL_CONSTANT_ATTENUATION, 0.55f);
                    glLightf(GL_LIGHT0 + light_index, GL_LINEAR_ATTENUATION, 0.070f);
                    glLightf(GL_LIGHT0 + light_index, GL_QUADRATIC_ATTENUATION, 0.012f);
                }

                light_index++;
            }
        }
    }

    for (i = 0; i < game->object_count; i++) {
        const MapObject* obj = &game->map_objects[i];

        Vec3 min;
        Vec3 max;

        if (obj->position.y > 0.01f) {
            min = vec3(
                obj->position.x - obj->scale.x * 0.5f,
                obj->position.y - obj->scale.y * 0.5f,
                obj->position.z - obj->scale.z * 0.5f
            );

            max = vec3(
                obj->position.x + obj->scale.x * 0.5f,
                obj->position.y + obj->scale.y * 0.5f,
                obj->position.z + obj->scale.z * 0.5f
            );
        } else {
            min = vec3(
                obj->position.x - obj->scale.x * 0.5f,
                0.0f,
                obj->position.z - obj->scale.z * 0.5f
            );

            max = vec3(
                obj->position.x + obj->scale.x * 0.5f,
                obj->scale.y,
                obj->position.z + obj->scale.z * 0.5f
            );
        }

        switch (obj->type) {
            case OBJ_BOUNDING_WALL:
            case OBJ_WALL_PANEL:
                if (wall_texture != 0) {
                    draw_textured_box(min, max, wall_texture, 2.0f);
                } else {
                    draw_box(min, max, 0.40f, 0.40f, 0.45f);
                }
                break;

            case OBJ_COLUMN:
                draw_box(min, max, 0.50f, 0.50f, 0.54f);
                break;

            case OBJ_PEDESTAL:
                draw_box(min, max, 0.58f, 0.58f, 0.62f);
                break;

            case OBJ_PLATFORM:
                if (floor_texture != 0) {
                    draw_textured_box(min, max, floor_texture, 2.0f);
                } else {
                    draw_box(min, max, 0.26f, 0.22f, 0.18f);
                }
                break;

            case OBJ_LOW_BLOCK:
                glDisable(GL_LIGHTING);
                glDisable(GL_CULL_FACE);

                if (ceiling_texture != 0) {
                    draw_textured_box_tinted(min, max, ceiling_texture, 2.0f, 0.30f, 0.23f, 0.16f);
                } else {
                    draw_box(min, max, 0.22f, 0.17f, 0.12f);
                }

                glEnable(GL_CULL_FACE);
                glEnable(GL_LIGHTING);
                break;

            default:
                break;
        }
    }

    if (game->win_counter >= 10) {
        glDisable(GL_LIGHTING);
        glDisable(GL_FOG);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_CULL_FACE);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        glPushMatrix();
        glTranslatef(game->exit_position.x, 2.1f, game->exit_position.z);

        glBegin(GL_TRIANGLE_FAN);
        glColor4f(1.0f, 0.82f, 0.35f, 0.90f);
        glVertex3f(0.0f, 0.0f, 0.0f);

        glColor4f(1.0f, 0.55f, 0.12f, 0.0f);
        for (i = 0; i <= 48; i++) {
            float a = (float)i / 48.0f * 2.0f * 3.14159265f;
            glVertex3f(0.0f, sinf(a) * 1.45f, cosf(a) * 1.65f);
        }
        glEnd();

        glPopMatrix();

        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
        glEnable(GL_FOG);
        glEnable(GL_LIGHTING);
    }

    for (i = 0; i < game->light_point_count; i++) {
        draw_light_pool(&game->light_points[i], pulse);
    }

    for (i = 0; i < game->light_point_count; i++) {
        draw_lamp_object(&game->light_points[i], pulse, lamp_model);
    }

    for (i = 0; i < game->light_point_count; i++) {
        draw_lamp_head_glow(game, &game->light_points[i], pulse);
    }

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

static GLuint font_big = 0;
static GLuint font_mid = 0;

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

static void init_menu_fonts(void)
{
    init_menu_font_list(&font_big, 74, "Georgia");
    init_menu_font_list(&font_mid, 30, "Georgia");
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

static void draw_text_px(GLuint font_list, const char* text, float x, float y, float r, float g, float b)
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

static void draw_centered_text_px(GLuint font_list, const char* text, float cx, float y, float approx_char_w, float r, float g, float b)
{
    float width;

    if (text == NULL || font_list == 0) {
        return;
    }

    width = (float)strlen(text) * approx_char_w;

    draw_text_px(font_list, text, cx - width * 0.5f, y, r, g, b);
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