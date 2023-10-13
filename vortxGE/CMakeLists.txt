cmake_minimum_required(VERSION 3.5)

project(vortxGE LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Bullet REQUIRED)

include_directories(${BULLET_INCLUDE_DIR})

file(GLOB_RECURSE MySources
    src/*.cpp
)

add_library(vortxGE STATIC
    ${MySources}
)

target_include_directories(vortxGE PUBLIC .)
