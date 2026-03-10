#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include "../include/render.h"

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
    glRotatef(-game->player.yaw - 90.0f, 0.0f, 1.0f, 0.0f);
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

static void draw_floor(void)
{
    glColor3f(0.28f, 0.28f, 0.30f);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-10.0f, 0.0f, -10.0f);
    glVertex3f(10.0f, 0.0f, -10.0f);
    glVertex3f(10.0f, 0.0f, 10.0f);
    glVertex3f(-10.0f, 0.0f, 10.0f);
    glEnd();
}

static void draw_ceiling(void)
{
    glColor3f(0.12f, 0.12f, 0.14f);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-10.0f, 3.0f, 10.0f);
    glVertex3f(10.0f, 3.0f, 10.0f);
    glVertex3f(10.0f, 3.0f, -10.0f);
    glVertex3f(-10.0f, 3.0f, -10.0f);
    glEnd();
}

static void draw_pedestal(Vec3 center)
{
    Vec3 min = {center.x - 0.5f, 0.0f, center.z - 0.5f};
    Vec3 max = {center.x + 0.5f, 1.2f, center.z + 0.5f};
    draw_box(min, max, 0.55f, 0.55f, 0.60f);

    glPushMatrix();
    glTranslatef(center.x, 1.65f, center.z);
    glColor3f(0.75f, 0.75f, 0.78f);
    glBegin(GL_QUADS);

    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-0.25f, 0.0f, -0.25f);
    glVertex3f(0.25f, 0.0f, -0.25f);
    glVertex3f(0.25f, 0.0f, 0.25f);
    glVertex3f(-0.25f, 0.0f, 0.25f);

    glEnd();
    glPopMatrix();
}

static void draw_light_marker(const LightPoint* light)
{
    float color = light->active ? 1.0f : 0.35f;

    glPushMatrix();
    glTranslatef(light->position.x, light->position.y, light->position.z);
    glColor3f(color, color, 0.4f);

    glBegin(GL_QUADS);
    glVertex3f(-0.25f, 0.25f, 0.0f);
    glVertex3f(0.25f, 0.25f, 0.0f);
    glVertex3f(0.25f, -0.25f, 0.0f);
    glVertex3f(-0.25f, -0.25f, 0.0f);

    glVertex3f(0.0f, 0.25f, -0.25f);
    glVertex3f(0.0f, 0.25f, 0.25f);
    glVertex3f(0.0f, -0.25f, 0.25f);
    glVertex3f(0.0f, -0.25f, -0.25f);
    glEnd();

    glPopMatrix();
}

void init_render_state(void)
{
    GLfloat ambient[] = {0.07f, 0.07f, 0.08f, 1.0f};
    GLfloat diffuse[] = {0.9f, 0.9f, 0.8f, 1.0f};
    GLfloat specular[] = {0.3f, 0.3f, 0.3f, 1.0f};

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_FOG);

    glFogf(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 4.0f);
    glFogf(GL_FOG_END, 18.0f);

    {
        GLfloat fog_color[] = {0.03f, 0.03f, 0.04f, 1.0f};
        glFogfv(GL_FOG_COLOR, fog_color);
    }

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

void render_scene(SDL_Window* window, const GameState* game)
{
    int w;
    int h;
    char title[256];
    GLfloat light_pos[] = {
        game->light_points[game->current_target].position.x,
        game->light_points[game->current_target].position.y,
        game->light_points[game->current_target].position.z,
        1.0f
    };
    GLfloat light_diffuse[] = {
        game->active_light_strength,
        game->active_light_strength,
        game->active_light_strength * 0.8f,
        1.0f
    };
    int i;

    SDL_GetWindowSize(window, &w, &h);

    snprintf(
        title,
        sizeof(title),
        "Dark Museum | timer: %.1f | reached: %d%s",
        game->darkness_timer,
        game->win_counter,
        game->game_over ? " | GAME OVER - Press R to restart" : ""
    );
    SDL_SetWindowTitle(window, title);

    glClearColor(0.02f, 0.02f, 0.03f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    apply_camera(game);

    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_diffuse);

    draw_floor();
    draw_ceiling();

    for (i = 0; i < game->collider_count; i++) {
        draw_box(game->colliders[i].min, game->colliders[i].max, 0.35f, 0.35f, 0.38f);
    }

    draw_pedestal(vec3(-4.0f, 0.0f, 4.8f));
    draw_pedestal(vec3(4.0f, 0.0f, -4.8f));
    draw_pedestal(vec3(0.0f, 0.0f, 0.0f));

    for (i = 0; i < game->light_point_count; i++) {
        draw_light_marker(&game->light_points[i]);
    }

    if (game->game_over) {
        glDisable(GL_LIGHTING);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glColor4f(0.5f, 0.0f, 0.0f, 0.35f);
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
        glEnable(GL_LIGHTING);
    }
}