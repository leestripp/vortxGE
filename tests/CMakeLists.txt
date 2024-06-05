cmake_minimum_required(VERSION 3.5)

project(tests LANGUAGES CXX)

find_package(Bullet REQUIRED)

# GLEW used by vortxGE
find_package(GLEW REQUIRED)

# openGL - LEGACY / GLVND
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

include_directories(
    src/
	../vortxGE/src/
    ${BULLET_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
)

file(GLOB MySources
    src/*.cpp
)

add_executable(tests
    ${MySources}
)

target_link_libraries(tests
    vortxGE
    ${BULLET_LIBRARIES}
    ${GLEW_LIBRARIES}
    OpenGL::EGL
)

add_dependencies(tests
	vortxGE
)

