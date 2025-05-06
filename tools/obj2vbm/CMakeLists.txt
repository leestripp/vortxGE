project(obj2vbm LANGUAGES CXX)

include_directories(
    src/
)

file(GLOB MySources
    src/convert/*.cpp
    src/filesystem/*.cpp
    src/math/*.cpp
    src/mesh/*.cpp

    src/*.cpp
)

add_executable(obj2vbm
    ${MySources}
)
