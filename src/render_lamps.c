#include <math.h>
#include <GL/gl.h>
#include "render_lamps.h"
#include "render_geometry.h"
#include "game.h"

#define LAMP_MODEL_BASE_OFFSET 1.78f
#define LAMP_HEAD_LIGHT_OFFSET 0.78f

void draw_lamp_object(const LightPoint* light, float pulse, GLuint lamp_model)
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

void draw_light_pool(const LightPoint* light, float pulse)
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

void draw_lamp_head_glow(const GameState* game, const LightPoint* light, float pulse)
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

