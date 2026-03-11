cmake_minimum_required(VERSION 3.20)
project(Dark_Museum C)

set(CMAKE_C_STANDARD 11)

set(SDK_ROOT "C:/Users/User/Downloads/c_sdk_220203/c_sdk_220203")
set(MINGW_ROOT "${SDK_ROOT}/MinGW")

include_directories(
        ${CMAKE_SOURCE_DIR}/include
        ${MINGW_ROOT}/include
        ${MINGW_ROOT}/include/SDL2
)

link_directories(
        ${MINGW_ROOT}/lib
)

add_executable(Dark_Museum
        main.c
        src/app.c
        src/game.c
        src/render.c
        src/collision.c
        src/math3d.c
)

target_link_libraries(Dark_Museum
        mingw32
        SDL2main
        SDL2
        opengl32
)