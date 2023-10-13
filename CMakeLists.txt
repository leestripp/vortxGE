cmake_minimum_required(VERSION 3.5)

project(vortxGE_all LANGUAGES CXX)

add_subdirectory(vortxGE)
add_subdirectory(demos/vortxGEdemo_sdl2)

ADD_CUSTOM_TARGET(link_target ALL COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_BINARY_DIR}/../demos/data" "${CMAKE_BINARY_DIR}/demos/vortxGEdemo_sdl2/data")
