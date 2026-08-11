project(vortxGE LANGUAGES CXX)

file(GLOB_RECURSE MySources
	src/*.cpp
)

add_library(vortxGE STATIC
	${MySources}
)

target_include_directories(vortxGE
	PUBLIC
	src/
	src/datablocks/tinygltf/
	# Generated config.h lives here (version derived from git tag —
	# see root CMakeLists.txt and src/config.h.in)
	${CMAKE_BINARY_DIR}/generated
)

# --- D-series (Plan.md): built-in debug & profiler tooling -----------------
# Everything behind VORTX_DEBUG_TOOLS compiles out completely in Release:
# no env-var strings, no branches, no bloat. Default: ON in Debug,
# OFF in Release; override with -DVORTX_DEBUG_TOOLS=ON/OFF.
if(CMAKE_BUILD_TYPE STREQUAL "Release")
	set(_vortx_debug_tools_default OFF)
else()
	set(_vortx_debug_tools_default ON)
endif()
option(VORTX_DEBUG_TOOLS "Built-in debug/profiler tooling (Plan.md D-series)" ${_vortx_debug_tools_default})
if(VORTX_DEBUG_TOOLS)
	target_compile_definitions(vortxGE PUBLIC VORTX_DEBUG_TOOLS=1)
endif()
message(STATUS "vortxGE debug tools (D-series): ${VORTX_DEBUG_TOOLS}")

# --- Vulkan backend (Plan.md Part 2: Vulkan primary) -----------------------
# The Vulkan backend (render/lsVulkan.{h,cpp}) needs the Vulkan loader and
# GLFW (glfwCreateWindowSurface). Linked PUBLIC so consumers (demos, tests)
# that pull in the static lib resolve glfwCreateWindowSurface etc.
find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)

target_link_libraries(vortxGE
	PUBLIC
	Vulkan::Vulkan
	glfw
	OmniNet
)

# glslang: runtime GLSL->SPIR-V compilation for the Vulkan backend
# (render/lsVulkanShader.cpp). Static libs from the glslang-dev package.
target_link_libraries(vortxGE
	PUBLIC
	glslang
	SPIRV
	glslang-default-resource-limits
	MachineIndependent
	OSDependent
	GenericCodeGen
	SPIRV-Tools-opt
	SPIRV-Tools
)
