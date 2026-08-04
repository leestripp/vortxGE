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
)

# glslang: runtime GLSL->SPIR-V compilation for the Vulkan backend
# (render/lsVulkanShader.cpp). Static libs from the glslang-dev package.
target_link_libraries(vortxGE
	PRIVATE
	glslang
	SPIRV
	glslang-default-resource-limits
)
