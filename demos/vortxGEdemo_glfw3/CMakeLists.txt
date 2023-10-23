cmake_minimum_required(VERSION 3.5)

project(vortxGEdemo_glfw3 LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Bullet REQUIRED)
find_package(PkgConfig REQUIRED)

# GLFW
pkg_search_module(GLFW REQUIRED glfw3)

# GLEW used by vortxGE
find_package(GLEW REQUIRED)

# openGL - LEGACY / GLVND
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)


include_directories(
    src/
    ../shared_code/
    ../../vortxGE/src/
    ${BULLET_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
)

file(GLOB MySources
    src/*.cpp
    ../shared_code/*.cpp
)

add_executable(vortxGEdemo_glfw3
    ${MySources}
)

target_link_libraries(vortxGEdemo_glfw3
    vortxGE
    ${BULLET_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${GLEW_LIBRARIES}
    OpenGL::EGL
)
