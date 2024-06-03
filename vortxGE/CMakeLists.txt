cmake_minimum_required(VERSION 3.5)

project(vortxGE LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
