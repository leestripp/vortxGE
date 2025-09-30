project(vortxGE LANGUAGES CXX)

include_directories(
	${BULLET_INCLUDE_DIR}
	${OPENGL_INCLUDE_DIR}
	${OPENAL_INCLUDE_DIR}
)

file(GLOB_RECURSE MySources
	src/*.cpp
)

add_library(vortxGE STATIC
	${MySources}
)

target_include_directories(vortxGE
	PUBLIC
	src/
	src/datablocks/tinygltf/
)
