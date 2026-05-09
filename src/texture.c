#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include "texture.h"

GLuint load_texture_bmp(const char* filename)
{
    SDL_Surface* loaded_surface;
    SDL_Surface* converted_surface;
    GLuint texture_id;
    unsigned char* pixels;
    int row_size;
    int y;

    loaded_surface = SDL_LoadBMP(filename);
    if (!loaded_surface) {
        fprintf(stderr, "Failed to load BMP texture: %s | %s\n", filename, SDL_GetError());
        return 0;
    }

    converted_surface = SDL_ConvertSurfaceFormat(loaded_surface, SDL_PIXELFORMAT_RGB24, 0);
    SDL_FreeSurface(loaded_surface);

    if (!converted_surface) {
        fprintf(stderr, "Failed to convert BMP texture: %s | %s\n", filename, SDL_GetError());
        return 0;
    }

    row_size = converted_surface->w * 3;
    pixels = (unsigned char*)malloc(row_size * converted_surface->h);

    if (!pixels) {
        fprintf(stderr, "Out of memory while loading texture: %s\n", filename);
        SDL_FreeSurface(converted_surface);
        return 0;
    }

    for (y = 0; y < converted_surface->h; y++) {
        memcpy(
            pixels + y * row_size,
            (unsigned char*)converted_surface->pixels + y * converted_surface->pitch,
            row_size
        );
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
        GL_RGB,
        converted_surface->w,
        converted_surface->h,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        pixels
    );

    glBindTexture(GL_TEXTURE_2D, 0);

    free(pixels);
    SDL_FreeSurface(converted_surface);

    printf("Loaded texture: %s\n", filename);

    return texture_id;
}