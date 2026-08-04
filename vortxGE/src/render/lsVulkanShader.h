/**
 * @file lsVulkanShader.h
 * @brief GLSL source transform + glslang compile for the Vulkan backend
 *
 * The engine's shaders are written for OpenGL: loose (non-block)
 * uniforms and unsized sampler declarations are legal there but not in
 * Vulkan GLSL. This unit performs a deterministic source transform at
 * compile time:
 *
 *  - every loose non-opaque `uniform T name;` is moved into a single
 *    std140 block at set=0/binding=0 (`_ubo`), usages rewritten
 *  - every sampler gets `layout(set=0, binding=K)` in declaration order
 *  - `#version` is normalised to 450
 *
 * The transform records each uniform's name/type/std140 offset so the
 * backend can serve GetUniformLocation + Uniform* writes into a CPU
 * staging buffer that is uploaded per draw.
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

/// One loose uniform moved into the default block.
struct lsVkUniformMember
{
	std::string name;
	std::string glslType;		// "float", "vec3", "mat4", "int", ...
	uint32_t	offset = 0;		// std140 byte offset inside the block
	uint32_t	arraySize = 0;	// 0 = scalar
	uint32_t	size = 0;		// bytes written by the matching Uniform* call
};

/// Result of transforming one shader stage.
struct lsVkShaderTransform
{
	std::string source;							// transformed GLSL
	std::vector<lsVkUniformMember> members;		// block members (ordered)
	std::vector<std::string> samplers;			// sampler names (binding = 1+index)
	uint32_t blockSize = 0;						// std140 block size in bytes
};

namespace lsVkShader
{
	/// Transform OpenGL-style GLSL for Vulkan consumption.
	lsVkShaderTransform transform( const std::string& source );

	/// Compile transformed GLSL to SPIR-V. stage: 0 = vertex, 1 = fragment.
	/// Returns SPIR-V words; empty on failure (log written to `log`).
	std::vector<uint32_t> compile( const std::string& source, int stage,
								   std::string& log );

	/// Process-wide glslang init/shutdown (idempotent init).
	void initCompiler();
	void shutdownCompiler();
}
