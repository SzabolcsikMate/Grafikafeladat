#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <GL/gl.h>

#define CGLTF_IMPLEMENTATION
#include "../include/cgltf.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

static void get_base_directory(const char* filepath, char* output, size_t output_size)
{
    const char* slash1 = strrchr(filepath, '/');
    const char* slash2 = strrchr(filepath, '\\');
    const char* slash = slash1;

    if (slash2 && (!slash1 || slash2 > slash1)) {
        slash = slash2;
    }

    if (!slash) {
        output[0] = '\0';
        return;
    }

    size_t length = (size_t)(slash - filepath + 1);

    if (length >= output_size) {
        length = output_size - 1;
    }

    memcpy(output, filepath, length);
    output[length] = '\0';
}

static GLuint load_texture_from_image(cgltf_image* image, const char* model_path)
{
    unsigned char* image_data = NULL;
    int width = 0;
    int height = 0;
    int channels = 0;
    GLuint texture_id = 0;

    if (!image) {
        return 0;
    }

    if (image->buffer_view) {
        const unsigned char* buffer_data;
        size_t buffer_size;

        buffer_data = (const unsigned char*)image->buffer_view->buffer->data
            + image->buffer_view->offset;

        buffer_size = image->buffer_view->size;

        image_data = stbi_load_from_memory(
            buffer_data,
            (int)buffer_size,
            &width,
            &height,
            &channels,
            4
        );
    }
    else if (image->uri) {
        char base_dir[512];
        char full_path[1024];

        get_base_directory(model_path, base_dir, sizeof(base_dir));

        snprintf(
            full_path,
            sizeof(full_path),
            "%s%s",
            base_dir,
            image->uri
        );

        image_data = stbi_load(
            full_path,
            &width,
            &height,
            &channels,
            4
        );
    }

    if (!image_data) {
        fprintf(stderr, "Failed to load GLB texture image.\n");
        return 0;
    }

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        image_data
    );

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(image_data);

    return texture_id;
}

static GLuint load_base_color_texture(cgltf_material* material, const char* model_path)
{
    cgltf_texture* texture;

    if (!material) {
        return 0;
    }

    texture = material->pbr_metallic_roughness.base_color_texture.texture;

    if (!texture || !texture->image) {
        return 0;
    }

    return load_texture_from_image(texture->image, model_path);
}

GLuint load_glb_model(const char* filepath)
{
    cgltf_options options = {0};
    cgltf_data* data = NULL;
    cgltf_result result;
    GLuint display_list;

    result = cgltf_parse_file(&options, filepath, &data);

    if (result != cgltf_result_success) {
        fprintf(stderr, "Failed to load GLB model: %s (Error code: %d)\n", filepath, result);
        return 0;
    }

    result = cgltf_load_buffers(&options, data, filepath);

    if (result != cgltf_result_success) {
        fprintf(stderr, "Failed to load GLB buffers: %s\n", filepath);
        cgltf_free(data);
        return 0;
    }

    display_list = glGenLists(1);
    glNewList(display_list, GL_COMPILE);

    if (data->meshes_count > 0) {
        cgltf_mesh* mesh = &data->meshes[0];

        for (cgltf_size i = 0; i < mesh->primitives_count; ++i) {
            cgltf_primitive* primitive = &mesh->primitives[i];
            cgltf_accessor* position_accessor = NULL;
            cgltf_accessor* normal_accessor = NULL;
            cgltf_accessor* texcoord_accessor = NULL;
            GLuint texture_id = 0;

            if (primitive->type != cgltf_primitive_type_triangles) {
                continue;
            }

            if (primitive->material) {
                cgltf_material* material = primitive->material;

                float r = material->pbr_metallic_roughness.base_color_factor[0];
                float g = material->pbr_metallic_roughness.base_color_factor[1];
                float b = material->pbr_metallic_roughness.base_color_factor[2];

                glColor3f(r, g, b);

                texture_id = load_base_color_texture(material, filepath);

                if (texture_id != 0) {
                    glEnable(GL_TEXTURE_2D);
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
                    glBindTexture(GL_TEXTURE_2D, texture_id);
                }
                else {
                    glDisable(GL_TEXTURE_2D);
                }
            }
            else {
                glColor3f(0.75f, 0.75f, 0.75f);
                glDisable(GL_TEXTURE_2D);
            }

            for (cgltf_size j = 0; j < primitive->attributes_count; ++j) {
                if (primitive->attributes[j].type == cgltf_attribute_type_position) {
                    position_accessor = primitive->attributes[j].data;
                }
                else if (primitive->attributes[j].type == cgltf_attribute_type_normal) {
                    normal_accessor = primitive->attributes[j].data;
                }
                else if (primitive->attributes[j].type == cgltf_attribute_type_texcoord) {
                    texcoord_accessor = primitive->attributes[j].data;
                }
            }

            if (!position_accessor) {
                continue;
            }

            glBegin(GL_TRIANGLES);

            if (primitive->indices) {
                cgltf_accessor* indices = primitive->indices;

                for (cgltf_size k = 0; k < indices->count; ++k) {
                    cgltf_uint index = cgltf_accessor_read_index(indices, k);

                    if (normal_accessor) {
                        float n[3];
                        cgltf_accessor_read_float(normal_accessor, index, n, 3);
                        glNormal3f(n[0], n[1], n[2]);
                    }

                    if (texcoord_accessor && texture_id != 0) {
                        float uv[2];
                        cgltf_accessor_read_float(texcoord_accessor, index, uv, 2);
                        glTexCoord2f(uv[0], 1.0f - uv[1]);
                    }

                    {
                        float v[3];
                        cgltf_accessor_read_float(position_accessor, index, v, 3);
                        glVertex3f(v[0], v[1], v[2]);
                    }
                }
            }
            else {
                for (cgltf_size k = 0; k < position_accessor->count; ++k) {
                    if (normal_accessor) {
                        float n[3];
                        cgltf_accessor_read_float(normal_accessor, k, n, 3);
                        glNormal3f(n[0], n[1], n[2]);
                    }

                    if (texcoord_accessor && texture_id != 0) {
                        float uv[2];
                        cgltf_accessor_read_float(texcoord_accessor, k, uv, 2);
                        glTexCoord2f(uv[0], 1.0f - uv[1]);
                    }

                    {
                        float v[3];
                        cgltf_accessor_read_float(position_accessor, k, v, 3);
                        glVertex3f(v[0], v[1], v[2]);
                    }
                }
            }

            glEnd();

            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
        }
    }

    glEndList();

    cgltf_free(data);

    return display_list;
}