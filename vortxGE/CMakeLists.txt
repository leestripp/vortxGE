cmake_minimum_required(VERSION 3.5)

project(vortxGE LANGUAGES CXX)

find_package(Bullet REQUIRED)
# openGL - LEGACY / GLVND
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)


include_directories(
    ${BULLET_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    )

file(GLOB_RECURSE MySources
    src/*.cpp
)

add_library(vortxGE STATIC
    ${MySources}
)

target_include_directories(vortxGE PUBLIC .)
