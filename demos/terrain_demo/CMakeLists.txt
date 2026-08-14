project(terrain_demo LANGUAGES CXX)

add_executable(terrain_demo
    src/main.cpp
    src/TerrainDemo.cpp
)

target_include_directories(terrain_demo PRIVATE
    ../../vortxGE/src/
)

target_link_libraries(terrain_demo
    vortxGE
)
