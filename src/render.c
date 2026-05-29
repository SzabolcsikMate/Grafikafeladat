#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include "render.h"
#include "ui.h"
#include "render_geometry.h"
#include "render_lamps.h"
#include "game.h"

#define LAMP_MODEL_BASE_OFFSET 1.78f
#define LAMP_HEAD_LIGHT_OFFSET 0.78f

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

    render_game_ui(game);
}

