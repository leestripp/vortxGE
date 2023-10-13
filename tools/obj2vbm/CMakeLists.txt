cmake_minimum_required(VERSION 3.5)

project(obj2vbm LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
