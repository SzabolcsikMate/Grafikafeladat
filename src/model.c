#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define CGLTF_IMPLEMENTATION
#include "../include/cgltf.h"

GLuint load_glb_model(const char* filepath)
{
    cgltf_options options = {0};
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, filepath, &data);

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

    GLuint display_list = glGenLists(1);
    glNewList(display_list, GL_COMPILE);

    /* Nagyon egyszerűsített GLB/glTF betöltő az első meshhez */
    if (data->meshes_count > 0) {
        cgltf_mesh* mesh = &data->meshes[0];
        for (cgltf_size i = 0; i < mesh->primitives_count; ++i) {
            cgltf_primitive* primitive = &mesh->primitives[i];

            if (primitive->type != cgltf_primitive_type_triangles) {
                continue;
            }

            cgltf_accessor* position_accessor = NULL;
            cgltf_accessor* normal_accessor = NULL;
            cgltf_accessor* texcoord_accessor = NULL;

            for (cgltf_size j = 0; j < primitive->attributes_count; ++j) {
                if (primitive->attributes[j].type == cgltf_attribute_type_position) {
                    position_accessor = primitive->attributes[j].data;
                } else if (primitive->attributes[j].type == cgltf_attribute_type_normal) {
                    normal_accessor = primitive->attributes[j].data;
                } else if (primitive->attributes[j].type == cgltf_attribute_type_texcoord) {
                    texcoord_accessor = primitive->attributes[j].data;
                }
            }

            if (!position_accessor) continue;

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

                    if (texcoord_accessor) {
                        float uv[2];
                        cgltf_accessor_read_float(texcoord_accessor, index, uv, 2);
                        glTexCoord2f(uv[0], uv[1]);
                    }

                    float v[3];
                    cgltf_accessor_read_float(position_accessor, index, v, 3);
                    glVertex3f(v[0], v[1], v[2]);
                }
            } else {
                for (cgltf_size k = 0; k < position_accessor->count; ++k) {
                    if (normal_accessor) {
                        float n[3];
                        cgltf_accessor_read_float(normal_accessor, k, n, 3);
                        glNormal3f(n[0], n[1], n[2]);
                    }

                    if (texcoord_accessor) {
                        float uv[2];
                        cgltf_accessor_read_float(texcoord_accessor, k, uv, 2);
                        glTexCoord2f(uv[0], uv[1]);
                    }

                    float v[3];
                    cgltf_accessor_read_float(position_accessor, k, v, 3);
                    glVertex3f(v[0], v[1], v[2]);
                }
            }

            glEnd();
        }
    }

    glEndList();
    cgltf_free(data);

    return display_list;
}
