project(tests LANGUAGES CXX)

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

