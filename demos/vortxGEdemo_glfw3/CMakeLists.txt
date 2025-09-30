project(vortxGEdemo_glfw3 LANGUAGES CXX)

find_package(PkgConfig REQUIRED)

# GLFW
pkg_search_module(GLFW REQUIRED glfw3)


include_directories(
    src/
    ../shared_code/
    ../../vortxGE/src/
    ${BULLET_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
    ${OPENAL_INCLUDE_DIRS}
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
    OpenGL::GL
    OpenAL::OpenAL
)

add_dependencies(vortxGEdemo_glfw3
	vortxGE
)
