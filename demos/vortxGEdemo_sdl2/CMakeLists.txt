cmake_minimum_required(VERSION 3.5)

project(vortxGEdemo_sdl2 LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Bullet REQUIRED)
find_package(SDL2 REQUIRED)

include_directories(
    src/
    ../../vortxGE/src/
    ${BULLET_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIRS}
)

file(GLOB MySources
    src/*.cpp
)

add_executable(vortxGEdemo_sdl2
    ${MySources}
)

target_link_libraries(vortxGEdemo_sdl2
    vortxGE
    ${BULLET_LIBRARIES}
    ${SDL2_LIBRARIES}
    GL
)
