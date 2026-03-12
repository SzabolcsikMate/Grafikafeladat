#include <stdio.h>
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include "texture.h"
#include <SDL_opengl.h>

#ifndef GL_BGR
#define GL_BGR 0x80E0
#endif

#ifndef GL_BGRA
#define GL_BGRA 0x80E1
#endif

GLuint load_texture_bmp(const char* filename)
{
    SDL_Surface* surface;
    GLuint texture_id;
    GLint format;

    surface = SDL_LoadBMP(filename);
    if (!surface) {
        fprintf(stderr, "Failed to load BMP texture: %s\n", filename);
        return 0;
    }

    if (surface->format->BytesPerPixel == 4) {
        format = GL_BGRA;
    } else if (surface->format->BytesPerPixel == 3) {
        format = GL_BGR;
    } else {
        fprintf(stderr, "Unsupported BMP format: %s\n", filename);
        SDL_FreeSurface(surface);
        return 0;
    }

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Some systems prefer GL_RGBA or GL_RGB as internal format
    GLint internalFormat = (surface->format->BytesPerPixel == 4) ? GL_RGBA : GL_RGB;

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        internalFormat,
        surface->w,
        surface->h,
        0,
        format,
        GL_UNSIGNED_BYTE,
        surface->pixels
    );

    SDL_FreeSurface(surface);
    return texture_id;
}

void bind_texture(GLuint texture_id)
{
    glBindTexture(GL_TEXTURE_2D, texture_id);
}