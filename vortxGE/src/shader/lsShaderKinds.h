#pragma once

// Shader kinds - centralized enum for type safety
enum class ShaderKind {
	None = 0,
	Debug,
	LightTexNone,
	LightTexD,
	LightTexDN,
	LightTexDNS,
	Occlusion,
	ShadelessTexD,
	ShadelessTexNone,
	Comp,
	Quad,
	Bloom,
	DebugNormals,
	ShadowMap,
	Skybox
};
