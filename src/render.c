#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include "render.h"

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
    glTranslatef(-game->player.position.x, -game->player.position.y, -game->player.position.z);
}

static void draw_box(Vec3 min, Vec3 max, float r, float g, float b)
{
    glColor3f(r, g, b);

    glBegin(GL_QUADS);

    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(min.x, max.y, min.z);
    glVertex3f(max.x, max.y, min.z);
    glVertex3f(max.x, max.y, max.z);
    glVertex3f(min.x, max.y, max.z);

    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(min.x, min.y, max.z);
    glVertex3f(max.x, min.y, max.z);
    glVertex3f(max.x, min.y, min.z);
    glVertex3f(min.x, min.y, min.z);

    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(min.x, min.y, max.z);
    glVertex3f(min.x, max.y, max.z);
    glVertex3f(max.x, max.y, max.z);
    glVertex3f(max.x, min.y, max.z);

    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(max.x, min.y, min.z);
    glVertex3f(max.x, max.y, min.z);
    glVertex3f(min.x, max.y, min.z);
    glVertex3f(min.x, min.y, min.z);

    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(min.x, min.y, min.z);
    glVertex3f(min.x, max.y, min.z);
    glVertex3f(min.x, max.y, max.z);
    glVertex3f(min.x, min.y, max.z);

    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(max.x, min.y, max.z);
    glVertex3f(max.x, max.y, max.z);
    glVertex3f(max.x, max.y, min.z);
    glVertex3f(max.x, min.y, min.z);

    glEnd();
}

static void draw_textured_box(Vec3 min, Vec3 max, GLuint texture_id, float tex_scale)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);

    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(min.x, max.y, min.z);
    glTexCoord2f(tex_scale, 0.0f); glVertex3f(max.x, max.y, min.z);
    glTexCoord2f(tex_scale, tex_scale); glVertex3f(max.x, max.y, max.z);
    glTexCoord2f(0.0f, tex_scale); glVertex3f(min.x, max.y, max.z);

    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(min.x, min.y, max.z);
    glTexCoord2f(tex_scale, 0.0f); glVertex3f(max.x, min.y, max.z);
    glTexCoord2f(tex_scale, tex_scale); glVertex3f(max.x, min.y, min.z);
    glTexCoord2f(0.0f, tex_scale); glVertex3f(min.x, min.y, min.z);

    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(min.x, min.y, max.z);
    glTexCoord2f(0.0f, tex_scale); glVertex3f(min.x, max.y, max.z);
    glTexCoord2f(tex_scale, tex_scale); glVertex3f(max.x, max.y, max.z);
    glTexCoord2f(tex_scale, 0.0f); glVertex3f(max.x, min.y, max.z);

    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(max.x, min.y, min.z);
    glTexCoord2f(0.0f, tex_scale); glVertex3f(max.x, max.y, min.z);
    glTexCoord2f(tex_scale, tex_scale); glVertex3f(min.x, max.y, min.z);
    glTexCoord2f(tex_scale, 0.0f); glVertex3f(min.x, min.y, min.z);

    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(min.x, min.y, min.z);
    glTexCoord2f(0.0f, tex_scale); glVertex3f(min.x, max.y, min.z);
    glTexCoord2f(tex_scale, tex_scale); glVertex3f(min.x, max.y, max.z);
    glTexCoord2f(tex_scale, 0.0f); glVertex3f(min.x, min.y, max.z);

    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(max.x, min.y, max.z);
    glTexCoord2f(0.0f, tex_scale); glVertex3f(max.x, max.y, max.z);
    glTexCoord2f(tex_scale, tex_scale); glVertex3f(max.x, max.y, min.z);
    glTexCoord2f(tex_scale, 0.0f); glVertex3f(max.x, min.y, min.z);

    glEnd();

    glDisable(GL_TEXTURE_2D);
}

static void draw_textured_floor(GLuint texture_id)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);

    glTexCoord2f(0.0f, 0.0f); glVertex3f(-10.0f, 0.0f, -10.0f);
    glTexCoord2f(8.0f, 0.0f); glVertex3f(10.0f, 0.0f, -10.0f);
    glTexCoord2f(8.0f, 8.0f); glVertex3f(10.0f, 0.0f, 10.0f);
    glTexCoord2f(0.0f, 8.0f); glVertex3f(-10.0f, 0.0f, 10.0f);

    glEnd();

    glDisable(GL_TEXTURE_2D);
}

static void draw_textured_ceiling(GLuint texture_id)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, -1.0f, 0.0f);

    glTexCoord2f(0.0f, 0.0f); glVertex3f(-10.0f, 3.0f, 10.0f);
    glTexCoord2f(8.0f, 0.0f); glVertex3f(10.0f, 3.0f, 10.0f);
    glTexCoord2f(8.0f, 8.0f); glVertex3f(10.0f, 3.0f, -10.0f);
    glTexCoord2f(0.0f, 8.0f); glVertex3f(-10.0f, 3.0f, -10.0f);

    glEnd();

    glDisable(GL_TEXTURE_2D);
}

static void draw_floor_grid(void)
{
    int i;

    glDisable(GL_LIGHTING);
    glColor3f(0.10f, 0.10f, 0.12f);

    glBegin(GL_LINES);

    for (i = -10; i <= 10; i++) {
        glVertex3f((float)i, 0.01f, -10.0f);
        glVertex3f((float)i, 0.01f, 10.0f);

        glVertex3f(-10.0f, 0.01f, (float)i);
        glVertex3f(10.0f, 0.01f, (float)i);
    }

    glEnd();

    glEnable(GL_LIGHTING);
}

static void draw_wall_panels(void)
{
    int i;

    for (i = -8; i <= 8; i += 4) {
        draw_box((Vec3){-9.45f, 0.8f, (float)i - 1.1f}, (Vec3){-9.35f, 2.1f, (float)i + 1.1f}, 0.45f, 0.43f, 0.38f);
        draw_box((Vec3){9.35f, 0.8f, (float)i - 1.1f}, (Vec3){9.45f, 2.1f, (float)i + 1.1f}, 0.45f, 0.43f, 0.38f);
    }
}

static void draw_pedestal(Vec3 center)
{
    Vec3 min = {center.x - 0.5f, 0.0f, center.z - 0.5f};
    Vec3 max = {center.x + 0.5f, 1.2f, center.z + 0.5f};

    draw_box(min, max, 0.58f, 0.58f, 0.62f);

    glPushMatrix();
    glTranslatef(center.x, 1.55f, center.z);
    glColor3f(0.82f, 0.82f, 0.85f);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-0.22f, 0.0f, -0.22f);
    glVertex3f(0.22f, 0.0f, -0.22f);
    glVertex3f(0.22f, 0.0f, 0.22f);
    glVertex3f(-0.22f, 0.0f, 0.22f);
    glEnd();

    glPopMatrix();
}

static void draw_column(Vec3 center)
{
    draw_box(
        (Vec3){center.x - 0.35f, 0.0f, center.z - 0.35f},
        (Vec3){center.x + 0.35f, 2.8f, center.z + 0.35f},
        0.50f, 0.50f, 0.54f
    );
}

static void draw_vitrine(Vec3 center)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.55f, 0.65f, 0.75f, 0.30f);

    glBegin(GL_QUADS);

    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(center.x - 0.6f, 0.2f, center.z + 0.6f);
    glVertex3f(center.x - 0.6f, 1.6f, center.z + 0.6f);
    glVertex3f(center.x + 0.6f, 1.6f, center.z + 0.6f);
    glVertex3f(center.x + 0.6f, 0.2f, center.z + 0.6f);

    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(center.x + 0.6f, 0.2f, center.z - 0.6f);
    glVertex3f(center.x + 0.6f, 1.6f, center.z - 0.6f);
    glVertex3f(center.x - 0.6f, 1.6f, center.z - 0.6f);
    glVertex3f(center.x - 0.6f, 0.2f, center.z - 0.6f);

    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(center.x - 0.6f, 0.2f, center.z - 0.6f);
    glVertex3f(center.x - 0.6f, 1.6f, center.z - 0.6f);
    glVertex3f(center.x - 0.6f, 1.6f, center.z + 0.6f);
    glVertex3f(center.x - 0.6f, 0.2f, center.z + 0.6f);

    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(center.x + 0.6f, 0.2f, center.z + 0.6f);
    glVertex3f(center.x + 0.6f, 1.6f, center.z + 0.6f);
    glVertex3f(center.x + 0.6f, 1.6f, center.z - 0.6f);
    glVertex3f(center.x + 0.6f, 0.2f, center.z - 0.6f);

    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(center.x - 0.6f, 1.6f, center.z - 0.6f);
    glVertex3f(center.x + 0.6f, 1.6f, center.z - 0.6f);
    glVertex3f(center.x + 0.6f, 1.6f, center.z + 0.6f);
    glVertex3f(center.x - 0.6f, 1.6f, center.z + 0.6f);

    glEnd();

    glDisable(GL_BLEND);

    draw_box(
        (Vec3){center.x - 0.62f, 0.0f, center.z - 0.62f},
        (Vec3){center.x + 0.62f, 0.2f, center.z + 0.62f},
        0.30f, 0.30f, 0.33f
    );
}

static void draw_light_marker(const LightPoint* light, float pulse)
{
    float sx = light->active ? 0.42f + 0.08f * pulse : 0.22f;
    float sy = light->active ? 0.42f + 0.08f * pulse : 0.22f;

    glPushMatrix();
    glTranslatef(light->position.x, light->position.y, light->position.z);

    if (light->active) {
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 0.95f, 0.25f);

        glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-sx, sy, 0.0f);
        glVertex3f(sx, sy, 0.0f);
        glVertex3f(sx, -sy, 0.0f);
        glVertex3f(-sx, -sy, 0.0f);

        glNormal3f(1.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, sy, -sx);
        glVertex3f(0.0f, sy, sx);
        glVertex3f(0.0f, -sy, sx);
        glVertex3f(0.0f, -sy, -sx);
        glEnd();

        glEnable(GL_LIGHTING);
    } else {
        glColor3f(0.35f, 0.35f, 0.20f);

        glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-sx, sy, 0.0f);
        glVertex3f(sx, sy, 0.0f);
        glVertex3f(sx, -sy, 0.0f);
        glVertex3f(-sx, -sy, 0.0f);

        glNormal3f(1.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, sy, -sx);
        glVertex3f(0.0f, sy, sx);
        glVertex3f(0.0f, -sy, sx);
        glVertex3f(0.0f, -sy, -sx);
        glEnd();
    }

    glPopMatrix();
}

void init_render_state(void)
{
    GLfloat ambient[] = {0.16f, 0.16f, 0.18f, 1.0f};
    GLfloat diffuse[] = {0.95f, 0.92f, 0.75f, 1.0f};
    GLfloat specular[] = {0.35f, 0.35f, 0.30f, 1.0f};
    GLfloat fog_color[] = {0.03f, 0.03f, 0.04f, 1.0f};

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_FOG);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glFogf(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 6.0f);
    glFogf(GL_FOG_END, 22.0f);
    glFogfv(GL_FOG_COLOR, fog_color);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
}

void resize_viewport(int width, int height)
{
    if (height <= 0) {
        height = 1;
    }

    glViewport(0, 0, width, height);
    set_perspective(70.0f, (float)width / (float)height, 0.1f, 50.0f);
}

void render_scene(SDL_Window* window, const GameState* game, GLuint floor_texture, GLuint wall_texture, GLuint ceiling_texture)
{
    int w;
    int h;
    int i;
    Uint32 ticks;
    float pulse;
    char title[256];

    GLfloat light_pos[] = {
        game->light_points[game->current_target].position.x,
        game->light_points[game->current_target].position.y,
        game->light_points[game->current_target].position.z,
        1.0f
    };

    GLfloat light_diffuse[] = {
        game->active_light_strength,
        game->active_light_strength * 0.95f,
        game->active_light_strength * 0.70f,
        1.0f
    };

    SDL_GetWindowSize(window, &w, &h);
    ticks = SDL_GetTicks();
    pulse = 0.5f + 0.5f * sinf((float)ticks * 0.006f);

    snprintf(
        title,
        sizeof(title),
        "Dark Museum | timer: %.1f | reached: %d%s",
        game->darkness_timer,
        game->win_counter,
        game->game_over ? " | GAME OVER - Press R or Enter to restart" : ""
    );
    SDL_SetWindowTitle(window, title);

    glClearColor(0.02f, 0.02f, 0.03f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    apply_camera(game);

    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_diffuse);

    if (floor_texture != 0) {
        draw_textured_floor(floor_texture);
    } else {
        draw_box((Vec3){-10.0f, -0.01f, -10.0f}, (Vec3){10.0f, 0.0f, 10.0f}, 0.22f, 0.22f, 0.24f);
    }

    draw_floor_grid();

    if (ceiling_texture != 0) {
        draw_textured_ceiling(ceiling_texture);
    } else {
        draw_box((Vec3){-10.0f, 3.0f, -10.0f}, (Vec3){10.0f, 3.01f, 10.0f}, 0.10f, 0.10f, 0.12f);
    }

    draw_wall_panels();

    for (i = 0; i < game->collider_count; i++) {
        // draw walls
        if (wall_texture != 0) {
            draw_textured_box(game->colliders[i].min, game->colliders[i].max, wall_texture, 2.0f);
        } else {
            draw_box(game->colliders[i].min, game->colliders[i].max, 0.42f, 0.42f, 0.46f);
        }
    }

    draw_column((Vec3){-7.0f, 0.0f, -1.5f});
    draw_column((Vec3){7.0f, 0.0f, 1.5f});
    draw_column((Vec3){-7.0f, 0.0f, 6.0f});
    draw_column((Vec3){7.0f, 0.0f, -6.0f});

    draw_pedestal((Vec3){-4.0f, 0.0f, 4.8f});
    draw_pedestal((Vec3){4.0f, 0.0f, -4.8f});
    draw_pedestal((Vec3){0.0f, 0.0f, -6.0f});
    draw_pedestal((Vec3){0.0f, 0.0f, 6.0f});

    draw_vitrine((Vec3){-7.2f, 0.0f, 3.4f});
    draw_vitrine((Vec3){7.2f, 0.0f, -3.4f});

    for (i = 0; i < game->light_point_count; i++) {
        draw_light_marker(&game->light_points[i], pulse);
    }

    if (game->game_over) {
        glDisable(GL_LIGHTING);
        glDisable(GL_FOG);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glColor4f(0.70f, 0.02f, 0.02f, 0.45f);
        glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(1.0f, 0.0f);
        glVertex2f(1.0f, 1.0f);
        glVertex2f(0.0f, 1.0f);
        glEnd();

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);

        glEnable(GL_FOG);
        glEnable(GL_LIGHTING);
    }
}