cmake_minimum_required(VERSION 3.5)

project(vortxGE_all LANGUAGES CXX)

add_subdirectory(vortxGE)
add_subdirectory(demos/vortxGEdemo_sdl2)
add_subdirectory(demos/vortxGEdemo_gtkmm)

ADD_CUSTOM_TARGET(link_sdl2 ALL COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_BINARY_DIR}/../demos/data" "${CMAKE_BINARY_DIR}/demos/vortxGEdemo_sdl2/data")
ADD_CUSTOM_TARGET(link_gtkmm ALL COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_BINARY_DIR}/../demos/data" "${CMAKE_BINARY_DIR}/demos/vortxGEdemo_gtkmm/data")
