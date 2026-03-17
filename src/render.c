#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include "render.h"
#include "game.h"

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

    glTexCoord2f(0.0f, 0.0f); glVertex3f(-15.0f, 0.0f, -15.0f);
    glTexCoord2f(15.0f, 0.0f); glVertex3f(15.0f, 0.0f, -15.0f);
    glTexCoord2f(15.0f, 15.0f); glVertex3f(15.0f, 0.0f, 15.0f);
    glTexCoord2f(0.0f, 15.0f); glVertex3f(-15.0f, 0.0f, 15.0f);

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

    glTexCoord2f(0.0f, 0.0f); glVertex3f(-15.0f, 3.0f, 15.0f);
    glTexCoord2f(15.0f, 0.0f); glVertex3f(15.0f, 3.0f, 15.0f);
    glTexCoord2f(15.0f, 15.0f); glVertex3f(15.0f, 3.0f, -15.0f);
    glTexCoord2f(0.0f, 15.0f); glVertex3f(-15.0f, 3.0f, -15.0f);

    glEnd();

    glDisable(GL_TEXTURE_2D);
}

static void draw_floor_grid(void)
{
    int i;

    glDisable(GL_LIGHTING);
    glColor3f(0.10f, 0.10f, 0.12f);

    glBegin(GL_LINES);

    for (i = -15; i <= 15; i++) {
        glVertex3f((float)i, 0.01f, -15.0f);
        glVertex3f((float)i, 0.01f, 15.0f);

        glVertex3f(-15.0f, 0.01f, (float)i);
        glVertex3f(15.0f, 0.01f, (float)i);
    }

    glEnd();

    glEnable(GL_LIGHTING);
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
    GLfloat specular[] = {0.35f, 0.35f, 0.30f, 1.0f};
    GLfloat fog_color[] = {0.03f, 0.03f, 0.04f, 1.0f};

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_FOG);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glFogf(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 6.0f);
    glFogf(GL_FOG_END, 22.0f);
    glFogfv(GL_FOG_COLOR, fog_color);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

    for (int i = 0; i < MAX_LIGHT_POINTS; ++i) {
        glEnable(GL_LIGHT0 + i);
        glLightfv(GL_LIGHT0 + i, GL_SPECULAR, specular);
    }
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

    for (i = 0; i < game->light_point_count; ++i) {
        GLfloat light_pos[] = {
            game->light_points[i].position.x,
            game->light_points[i].position.y,
            game->light_points[i].position.z,
            1.0f
        };

        float intensity = game->light_points[i].base_intensity;
        if (i == game->current_target) {
            intensity += game->active_light_strength;
        }

        GLfloat light_diffuse[] = {
            intensity,
            intensity * 0.95f,
            intensity * 0.70f,
            1.0f
        };

        glLightfv(GL_LIGHT0 + i, GL_POSITION, light_pos);
        glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, light_diffuse);
    }

    if (floor_texture != 0) {
        draw_textured_floor(floor_texture);
    } else {
        draw_box((Vec3){-15.0f, -0.01f, -15.0f}, (Vec3){15.0f, 0.0f, 15.0f}, 0.22f, 0.22f, 0.24f);
    }

    draw_floor_grid();

    if (ceiling_texture != 0) {
        draw_textured_ceiling(ceiling_texture);
    } else {
        draw_box((Vec3){-15.0f, 3.0f, -15.0f}, (Vec3){15.0f, 3.01f, 15.0f}, 0.10f, 0.10f, 0.12f);
    }

    for (i = 0; i < game->collider_count; i++) {
        if (wall_texture != 0) {
            draw_textured_box(game->colliders[i].min, game->colliders[i].max, wall_texture, 2.0f);
        } else {
            draw_box(game->colliders[i].min, game->colliders[i].max, 0.42f, 0.42f, 0.46f);
        }
    }

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