project(vortxGEdemo_sdl2 LANGUAGES CXX)

find_package(SDL2 REQUIRED)


include_directories(
    src/
    ../shared_code/
    ../../vortxGE/src/
    ${BULLET_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
)

file(GLOB MySources
    src/*.cpp
    ../shared_code/*.cpp
)

add_executable(vortxGEdemo_sdl2
    ${MySources}
)

target_link_libraries(vortxGEdemo_sdl2
    vortxGE
    ${BULLET_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${GLEW_LIBRARIES}
    OpenGL::EGL
)

add_dependencies(vortxGEdemo_sdl2
	vortxGE
)
