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
    glColor3f(1.0f, 1.0f, 1.0f);

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

    glDisable(GL_TEXTURE_2D);
}

static void draw_textured_floor(GLuint texture_id)
{
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f);  glVertex3f(-18.0f, 0.0f, 18.0f);
    glTexCoord2f(12.0f, 0.0f); glVertex3f(18.0f, 0.0f, 18.0f);
    glTexCoord2f(12.0f, 12.0f);glVertex3f(18.0f, 0.0f, -18.0f);
    glTexCoord2f(0.0f, 12.0f); glVertex3f(-18.0f, 0.0f, -18.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

static void draw_textured_ceiling(GLuint texture_id)
{
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f);  glVertex3f(-18.0f, 3.0f, -18.0f);
    glTexCoord2f(12.0f, 0.0f); glVertex3f(18.0f, 3.0f, -18.0f);
    glTexCoord2f(12.0f, 12.0f);glVertex3f(18.0f, 3.0f, 18.0f);
    glTexCoord2f(0.0f, 12.0f); glVertex3f(-18.0f, 3.0f, 18.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

static void draw_vitrine_translucent(Vec3 min, Vec3 max)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.6f, 0.8f, 0.9f, 0.28f);

    glBegin(GL_QUADS);

    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(min.x, min.y, max.z); glVertex3f(max.x, min.y, max.z);
    glVertex3f(max.x, max.y, max.z); glVertex3f(min.x, max.y, max.z);

    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(max.x, min.y, min.z); glVertex3f(min.x, min.y, min.z);
    glVertex3f(min.x, max.y, min.z); glVertex3f(max.x, max.y, min.z);

    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(min.x, min.y, min.z); glVertex3f(min.x, min.y, max.z);
    glVertex3f(min.x, max.y, max.z); glVertex3f(min.x, max.y, min.z);

    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(max.x, min.y, max.z); glVertex3f(max.x, min.y, min.z);
    glVertex3f(max.x, max.y, min.z); glVertex3f(max.x, max.y, max.z);

    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(min.x, max.y, max.z); glVertex3f(max.x, max.y, max.z);
    glVertex3f(max.x, max.y, min.z); glVertex3f(min.x, max.y, min.z);

    glEnd();

    glDisable(GL_BLEND);

    draw_box(vec3(min.x - 0.05f, 0.0f, min.z - 0.05f), vec3(max.x + 0.05f, 0.2f, max.z + 0.05f), 0.2f, 0.2f, 0.22f);
}

static void draw_light_marker(const LightPoint* light, float pulse)
{
    float sx = light->active ? (0.28f + 0.08f * pulse) : 0.16f;

    glPushMatrix();
    glTranslatef(light->position.x, light->position.y, light->position.z);

    if (light->active) {
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 0.95f, 0.45f);
    } else {
        glDisable(GL_LIGHTING);
        glColor3f(0.35f, 0.35f, 0.25f);
    }

    glBegin(GL_QUADS);
    glVertex3f(-sx, -sx, 0.0f); glVertex3f(sx, -sx, 0.0f);
    glVertex3f(sx, sx, 0.0f);   glVertex3f(-sx, sx, 0.0f);

    glVertex3f(0.0f, -sx, sx);  glVertex3f(0.0f, -sx, -sx);
    glVertex3f(0.0f, sx, -sx);  glVertex3f(0.0f, sx, sx);
    glEnd();

    glEnable(GL_LIGHTING);
    glPopMatrix();
}

void init_render_state(void)
{
    GLfloat ambient[] = {0.12f, 0.12f, 0.14f, 1.0f};
    GLfloat specular[] = {0.20f, 0.20f, 0.20f, 1.0f};
    GLfloat fog_color[] = {0.03f, 0.03f, 0.04f, 1.0f};

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 7.0f);
    glFogf(GL_FOG_END, 26.0f);
    glFogfv(GL_FOG_COLOR, fog_color);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

    glEnable(GL_LIGHT0);

    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.8f);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.18f);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.05f);
}

void resize_viewport(int width, int height)
{
    if (height <= 0) {
        height = 1;
    }

    glViewport(0, 0, width, height);
    set_perspective(70.0f, (float)width / (float)height, 0.1f, 60.0f);
}

void render_scene(SDL_Window* window, const GameState* game, GLuint floor_texture, GLuint wall_texture, GLuint ceiling_texture)
{
    int i;
    Uint32 ticks;
    float pulse;
    char title[256];

    ticks = SDL_GetTicks();
    pulse = 0.5f + 0.5f * sinf((float)ticks * 0.006f);

    snprintf(
        title,
        sizeof(title),
        "Dark Museum | Relics Found: %d%s",
        game->win_counter,
        game->game_over ? " | GAME OVER - Press R" : ""
    );
    SDL_SetWindowTitle(window, title);

    glClearColor(0.02f, 0.02f, 0.03f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    apply_camera(game);

    if (game->light_point_count > 0) {
        int target_idx = game->current_target;
        GLfloat light_pos[] = {
            game->light_points[target_idx].position.x,
            game->light_points[target_idx].position.y,
            game->light_points[target_idx].position.z,
            1.0f
        };

        /* JAVÍTVA: A game->active_light_strength-re már nincs szükségünk,
           hiszen a light_points tömb minden eleme tartalmazza a saját intensity-jét */
        GLfloat intensity = game->light_points[target_idx].current_intensity + pulse * 0.15f;
        GLfloat light_diffuse[] = {
            intensity,
            intensity * 0.92f,
            intensity * 0.72f,
            1.0f
        };

        glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, light_diffuse);
    }

    if (floor_texture != 0) {
        draw_textured_floor(floor_texture);
    } else {
        draw_box(vec3(-18.0f, -0.01f, -18.0f), vec3(18.0f, 0.0f, 18.0f), 0.25f, 0.25f, 0.27f);
    }

    if (ceiling_texture != 0) {
        draw_textured_ceiling(ceiling_texture);
    } else {
        draw_box(vec3(-18.0f, 3.0f, -18.0f), vec3(18.0f, 3.01f, 18.0f), 0.18f, 0.18f, 0.20f);
    }

    for (i = 0; i < game->object_count; i++) {
        const MapObject* obj = &game->map_objects[i];
        Vec3 min = vec3(obj->position.x - obj->scale.x * 0.5f, 0.0f, obj->position.z - obj->scale.z * 0.5f);
        Vec3 max = vec3(obj->position.x + obj->scale.x * 0.5f, obj->scale.y, obj->position.z + obj->scale.z * 0.5f);

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
            case OBJ_VITRINE:
                draw_vitrine_translucent(min, max);
                break;
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

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.80f, 0.02f, 0.02f, 0.55f);
        glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f); glVertex2f(1.0f, 0.0f);
        glVertex2f(1.0f, 1.0f); glVertex2f(0.0f, 1.0f);
        glEnd();

        glDisable(GL_BLEND);

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);

        glEnable(GL_FOG);
        glEnable(GL_LIGHTING);
    }
}