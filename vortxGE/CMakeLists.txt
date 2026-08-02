project(vortxGE LANGUAGES CXX)

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
	# Generated config.h lives here (version derived from git tag —
	# see root CMakeLists.txt and src/config.h.in)
	${CMAKE_BINARY_DIR}/generated
)
