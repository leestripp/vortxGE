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
)
