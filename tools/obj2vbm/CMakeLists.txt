project(obj2vbm LANGUAGES CXX)

include_directories(
	../../vortxGE/src/
    src/
)

file(GLOB MySources
    ../../vortxGE/src/math/*.cpp
    src/convert/*.cpp
    src/filesystem/*.cpp
    src/mesh/*.cpp
	
    src/*.cpp
)

add_executable(obj2vbm
    ${MySources}
)
