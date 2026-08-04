cmake_minimum_required(VERSION 3.29)

# --- Version from git tag ---------------------------------------------------
# The project version is derived from the latest git tag (e.g. "0.1.0").
# To cut a release: create a semver git tag and re-run CMake — no code
# edits needed. When git metadata is unavailable (tarball/zip build) the
# fallback below is used instead. Keep the fallback at the latest release.
set(vortxGE_VERSION_MAJOR 0)   # --- fallback version ---
set(vortxGE_VERSION_MINOR 1)
set(vortxGE_VERSION_PATCH 0)
set(vortxGE_VERSION_GIT "unknown")

find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --always --dirty
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_DESCRIBE
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_LATEST_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    # Accept "1.2.3" and tolerate a leading "v" ("v1.2.3")
    if(GIT_LATEST_TAG MATCHES "^v?([0-9]+)\\.([0-9]+)\\.([0-9]+)")
        set(vortxGE_VERSION_MAJOR "${CMAKE_MATCH_1}")
        set(vortxGE_VERSION_MINOR "${CMAKE_MATCH_2}")
        set(vortxGE_VERSION_PATCH "${CMAKE_MATCH_3}")
        if(GIT_DESCRIBE)
            set(vortxGE_VERSION_GIT "${GIT_DESCRIBE}")
        endif()
    elseif(GIT_LATEST_TAG)
        message(WARNING
            "Latest git tag '${GIT_LATEST_TAG}' is not semver (x.y.z); "
            "using fallback version "
            "${vortxGE_VERSION_MAJOR}.${vortxGE_VERSION_MINOR}.${vortxGE_VERSION_PATCH}")
    endif()
endif()

set(vortxGE_VERSION_STRING
    "${vortxGE_VERSION_MAJOR}.${vortxGE_VERSION_MINOR}.${vortxGE_VERSION_PATCH}")

project(vortxGE_all
    VERSION ${vortxGE_VERSION_STRING}
    LANGUAGES CXX
)

message(STATUS "vortxGE version: ${vortxGE_VERSION_STRING} (git describe: ${vortxGE_VERSION_GIT})")

# Generate config.h from the template — vortxGE/src/vortxGE.cpp includes it.
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/vortxGE/src/config.h.in
    ${CMAKE_BINARY_DIR}/generated/config.h
    @ONLY
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
#  set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_STANDARD 23)   # Phase 6: C++23 migration (Plan.md)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# bullet physics
find_package(Bullet REQUIRED)

# openGL - LEGACY / GLVND
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

# OpenAL
find_package(OpenAL REQUIRED)

# GLEW used by vortxGE
find_package(GLEW REQUIRED)

include_directories(
	${BULLET_INCLUDE_DIR}
	${OPENGL_INCLUDE_DIR}
	${OPENAL_INCLUDE_DIR}
)

# OmniNet — HTTP/WebSocket server library (networking for engine services:
# remote inspection, MCP-over-HTTP debug hooks, future multiplayer support).
# Components (tests/demos/benchmarks/tools) auto-disable when consumed via
# FetchContent (OmniNet >= b6e8084); sqlite-vec is not needed here.
include(FetchContent)
set(OMNINET_ENABLE_VEC OFF CACHE BOOL "vortxGE does not need sqlite-vec" FORCE)
FetchContent_Declare(omninet
	GIT_REPOSITORY https://gitlab.com/leestripp/omninet.git
	GIT_TAG        b6e808448e1f23757cf29b3442181618a8ecd024	# pin for reproducible builds
)
FetchContent_MakeAvailable(omninet)

add_subdirectory(vortxGE)

# CTest — test suite lives in tests/ (Plan.md Phase 0)
enable_testing()
add_subdirectory(tests)
add_subdirectory(demos/vortxGEdemo_gtkmm)
add_subdirectory(demos/vortxGEdemo_glfw3)

# build game
set(GAME_DIR "game")
# Check if the directory exists using the 'EXISTS' command
if(EXISTS "${GAME_DIR}")
	message(STATUS "Found optional subdirectory: ${GAME_DIR}. Including it in the build.")
	add_subdirectory(${GAME_DIR})
else()
	# If the directory does not exist, provide a message for clarity
	message(STATUS "Optional subdirectory '${GAME_DIR}' not found. Skipping build for game module.")
endif()

# Add links for data.
ADD_CUSTOM_TARGET(symlink_gtkmm ALL COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_BINARY_DIR}/../demos/data" "${CMAKE_BINARY_DIR}/demos/vortxGEdemo_gtkmm/data")
ADD_CUSTOM_TARGET(symlink_glfw3 ALL COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_BINARY_DIR}/../demos/data" "${CMAKE_BINARY_DIR}/demos/vortxGEdemo_glfw3/data")
