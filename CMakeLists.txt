cmake_minimum_required(VERSION 3.29)

project(vortxGE_all LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
#  set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# bullet physics
find_package(Bullet REQUIRED)

# openGL - LEGACY / GLVND
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

# GLEW used by vortxGE
find_package(GLEW REQUIRED)

add_subdirectory(vortxGE)
add_subdirectory(tests)
add_subdirectory(tools/obj2vbm)
add_subdirectory(demos/vortxGEdemo_gtkmm)
add_subdirectory(demos/vortxGEdemo_glfw3)

# Add links for data.
ADD_CUSTOM_TARGET(symlink_gtkmm ALL COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_BINARY_DIR}/../demos/data" "${CMAKE_BINARY_DIR}/demos/vortxGEdemo_gtkmm/data")
ADD_CUSTOM_TARGET(symlink_glfw3 ALL COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_BINARY_DIR}/../demos/data" "${CMAKE_BINARY_DIR}/demos/vortxGEdemo_glfw3/data")
