cmake_minimum_required(VERSION 3.5)

project(vortxGEdemo_gtkmm LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Bullet REQUIRED)

# gtkmm
find_package(PkgConfig)
pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)
# Now the variables GTKMM_INCLUDE_DIRS, GTKMM_LIBRARY_DIRS and GTKMM_LIBRARIES
link_directories(${GTKMM_LIBRARY_DIRS})

# glibmm
pkg_check_modules(GLIBMM REQUIRED glibmm-2.68)
# Now the variables GLIBMM_INCLUDE_DIRS, GLIBMM_LIBRARY_DIRS and GLIBMM_LIBRARIES
link_directories(${GLIBMM_LIBRARY_DIRS})

# GLEW used by vortxGE
find_package(GLEW REQUIRED)

# openGL - LEGACY / GLVND
set(OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL REQUIRED)

include_directories(
    src/
    ../../vortxGE/src/
    ${BULLET_INCLUDE_DIR}
    ${GTKMM_INCLUDE_DIRS}
    ${GLIBMM_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
)

file(GLOB MySources
    src/*.cpp
)

add_executable(vortxGEdemo_gtkmm
    ${MySources}
)

target_link_libraries(vortxGEdemo_gtkmm
    vortxGE
    ${BULLET_LIBRARIES}
    ${GTKMM_LIBRARIES}
    ${GLIBMM_LIBRARIES}
    ${GLEW_LIBRARIES}
    OpenGL::EGL
)
