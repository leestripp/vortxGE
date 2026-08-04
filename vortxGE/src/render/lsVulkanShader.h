/**
 * @file lsVulkanShader.h
 * @brief GLSL source transform + glslang compile for the Vulkan backend
 *
 * The engine's shaders are written for OpenGL: loose (non-block)
 * uniforms and unsized sampler declarations are legal there but not in
 * Vulkan GLSL. This unit transforms them at compile time:
 *
 *  - every loose non-opaque `uniform T name;` moves into a std140 block
 *    at set=0/binding=0 (`_ubo`); usages rewritten
 *  - every sampler gets `layout(set=0, binding=K)`
 *  - `#version` normalised to 450
 *
 * Pipeline programs link a vertex + fragment stage. To guarantee one
 * uniform gets the SAME block offset in both stages, the flow is:
 *
 *   parse(stage source)          → members/samplers (no offsets yet)
 *   mergeLayouts(vs, fs)         → canonical member list + offsets
 *   assemble(stage, layout)      → final GLSL with the shared block
 *   compile(final GLSL)          → SPIR-V
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
	uint32_t	offset = 0;		// std140 byte offset (assigned at merge)
	uint32_t	arraySize = 0;	// 0 = scalar
	uint32_t	size = 0;		// bytes the matching Uniform* call writes
};

/// One global in/out interface variable (no explicit layout in source).
struct lsVkInterfaceVar
{
	std::string name;
	uint32_t	arraySize = 0;	// 0 = scalar
	uint32_t	location = 0;	// assigned by assignInterface()
};

/// Per-stage parse result (pre-merge).
struct lsVkStageParse
{
	std::string body;							// source minus uniform decls, usages rewritten
	std::vector<lsVkUniformMember> members;		// loose uniforms found (offsets unset)
	std::vector<std::string> samplers;			// sampler names in declaration order
	std::vector<std::string> samplerTypes;		// their GLSL types (parallel)
	std::vector<uint32_t> samplerArrays;		// array sizes (0 = scalar, parallel)
	std::vector<lsVkInterfaceVar> ins;			// global `in` decls lacking layout(location)
	std::vector<lsVkInterfaceVar> outs;			// global `out` decls lacking layout(location)
};

/// Canonical program layout (post-merge).
struct lsVkProgramLayout
{
	std::vector<lsVkUniformMember> members;		// offsets assigned
	std::vector<std::string> samplers;			// merged, binding = 1 + index
	std::vector<uint32_t> samplerArrays;		// array sizes (0 = scalar, parallel)
	uint32_t blockSize = 0;
};

namespace lsVkShader
{
	/// Parse one stage: strip loose-uniform decls, rewrite usages to
	/// `_ubo.<name>`, collect members + samplers.
	lsVkStageParse parse( const std::string& source );

	/// Merge two stages' uniform sets into one canonical layout with
	/// std140 offsets (VS members first, FS-only members appended;
	/// duplicates keep the VS slot). Samplers merged the same way.
	lsVkProgramLayout mergeLayouts( const lsVkStageParse& vs,
									const lsVkStageParse& fs );

	/// Assign explicit interface locations (Vulkan requires them):
	/// VS outs numbered sequentially (arrays consume consecutive
	/// locations); FS ins matched to VS outs by NAME (unmatched get the
	/// next free locations); FS outs numbered from 0. VS ins keep their
	/// source `layout(location=N)` qualifiers.
	void assignInterface( lsVkStageParse& vs, lsVkStageParse& fs );

	/// Produce final compilable GLSL for a stage: #version 450 + shared
	/// block + this stage's sampler bindings + its body. In/out decls
	/// recorded by parse() get `layout(location=N)` injected.
	std::string assemble( const lsVkStageParse& stage,
						  const lsVkProgramLayout& layout );

	/// Compile GLSL to SPIR-V. stage: 0 = vertex, 1 = fragment.
	std::vector<uint32_t> compile( const std::string& source, int stage,
								   std::string& log );

	void initCompiler();
	void shutdownCompiler();
}
