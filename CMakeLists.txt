cmake_minimum_required(VERSION 3.20)
project(Dark_Museum C)

set(CMAKE_C_STANDARD 11)

set(SDL2_ROOT "C:/SDL2/x86_64-w64-mingw32")

add_executable(Dark_Museum
        main.c
        src/app.c
        src/game.c
        src/render.c
        src/collision.c
        src/math3d.c
        src/texture.c
)

target_include_directories(Dark_Museum PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${SDL2_ROOT}/include
)

target_link_directories(Dark_Museum PRIVATE
        ${SDL2_ROOT}/lib
)

target_link_libraries(Dark_Museum
        mingw32
        SDL2main
        SDL2
        opengl32
)