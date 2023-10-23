cmake_minimum_required(VERSION 3.5)

project(vortxGE_all LANGUAGES CXX)

add_subdirectory(vortxGE)
add_subdirectory(demos/vortxGEdemo_gtkmm)
add_subdirectory(demos/vortxGEdemo_glfw3)
add_subdirectory(demos/vortxGEdemo_sdl2)

ADD_CUSTOM_TARGET(symlink_gtkmm ALL COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_BINARY_DIR}/../demos/data" "${CMAKE_BINARY_DIR}/demos/vortxGEdemo_gtkmm/data")
ADD_CUSTOM_TARGET(symlink_glfw3 ALL COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_BINARY_DIR}/../demos/data" "${CMAKE_BINARY_DIR}/demos/vortxGEdemo_glfw3/data")
ADD_CUSTOM_TARGET(symlink_sdl2 ALL COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_BINARY_DIR}/../demos/data" "${CMAKE_BINARY_DIR}/demos/vortxGEdemo_sdl2/data")
