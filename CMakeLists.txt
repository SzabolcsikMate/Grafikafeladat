cmake_minimum_required(VERSION 3.20)
project(Dark_Museum C)

set(CMAKE_C_STANDARD 11)

find_package(OpenGL REQUIRED)

set(SOURCE_FILES
        main.c
        src/app.c
        src/game.c
        src/map.c
        src/lamps.c
        src/player.c
        src/render.c
        src/render_geometry.c
        src/render_lamps.c
        src/ui.c
        src/font.c
        src/collision.c
        src/math3d.c
        src/texture.c
        src/model.c
)

add_executable(Dark_Museum ${SOURCE_FILES})

target_include_directories(Dark_Museum PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

if (WIN32 AND EXISTS "${CMAKE_SOURCE_DIR}/../MinGW")
    set(MINGW_ROOT "${CMAKE_SOURCE_DIR}/../MinGW")

    target_include_directories(Dark_Museum PRIVATE
            ${MINGW_ROOT}/include
            ${MINGW_ROOT}/include/SDL2
    )

    target_link_directories(Dark_Museum PRIVATE
            ${MINGW_ROOT}/lib
    )

    target_link_libraries(Dark_Museum
            mingw32
            SDL2main
            SDL2
            opengl32
            glu32
            gdi32
    )
else()
    find_package(SDL2 REQUIRED)

    target_include_directories(Dark_Museum PRIVATE
            ${SDL2_INCLUDE_DIRS}
    )

    target_link_libraries(Dark_Museum
            ${SDL2_LIBRARIES}
            OpenGL::GL
            m
    )
endif()