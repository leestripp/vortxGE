project(tests LANGUAGES CXX)

include_directories(
    src/
	../vortxGE/src/
    third_party/            # vendored doctest (tests/third_party/doctest/doctest.h)
    ${BULLET_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
)

file(GLOB_RECURSE MySources
    src/*.cpp
)

add_executable(tests
    ${MySources}
)

target_link_libraries(tests
    vortxGE
    ${BULLET_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${OPENAL_LIBRARY}         # scene/ classes (lsListener) pull in OpenAL
    OpenGL::EGL
)

add_dependencies(tests
	vortxGE
)

# CTest registration — run with: ctest --output-on-failure
add_test(NAME vortxGE_tests COMMAND tests)

