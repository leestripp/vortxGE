cmake_minimum_required(VERSION 3.5)

project(vortxGEdemo_gtkmm LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Bullet REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM_VARS REQUIRED IMPORTED_TARGET gtkmm-4.0)

include_directories(
    src/
    ../../vortxGE/src/
    ${BULLET_INCLUDE_DIR}
    ${GTKMM_INCLUDE_DIRS}
)

file(GLOB MySources
    src/*.cpp
)

add_executable(vortxGEdemo_gtkmm
    ${MySources}
)

target_link_libraries(vortxGEdemo_gtkmm
    vortxGE
    ${BULLET_LIBRARIES}
    ${GTKMM_LIBRARIES}
    GL
)
