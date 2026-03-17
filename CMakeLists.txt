cmake_minimum_required(VERSION 3.20)
project(Dark_Museum C)

set(CMAKE_C_STANDARD 11)

set(SDL2_ROOT "C:/SDL2/SDL2-devel-2.32.10-mingw/SDL2-2.32.10/x86_64-w64-mingw32")

include_directories(
        ${CMAKE_SOURCE_DIR}/include
        ${SDL2_ROOT}/include
)

link_directories(
        ${SDL2_ROOT}/lib
)

add_executable(Dark_Museum
        main.c
        src/app.c
        src/game.c
        src/render.c
        src/collision.c
        src/math3d.c
        src/texture.c
)

target_link_libraries(Dark_Museum
        mingw32
        SDL2main
        SDL2
        opengl32
        m
)
