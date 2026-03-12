/**
 * @file lsShaderKinds.h
 * @brief Shader type enumerations
 * 
 * Centralized enum definitions for shader types used throughout the engine.
 * Provides type safety and consistent naming for different shader programs.
 * 
 * @author vortxGE
 * @date 2024
 */

#pragma once

/**
 * @enum ShaderKind
 * @brief Enumeration of all shader types in the engine
 * 
 * Each ShaderKind represents a specific shader program used for
 * different rendering techniques.
 * 
 * @var ShaderKind::None
 *      No shader - used for placeholder or disabled rendering
 * 
 * @var ShaderKind::Debug
 *      Debug shader for general debugging visualization
 * 
 * @var ShaderKind::LightTexNone
 *      Lighting shader with texture, no normal mapping
 * 
 * @var ShaderKind::LightTexD
 *      Lighting shader with diffuse texture only
 * 
 * @var ShaderKind::LightTexDN
 *      Lighting shader with diffuse and normal mapping
 * 
 * @var ShaderKind::LightTexDNS
 *      Lighting shader with diffuse, normal, and specular mapping
 * 
 * @var ShaderKind::Occlusion
 *      Occlusion query shader for visibility testing
 * 
 * @var ShaderKind::ShadelessTexD
 *      Unlit shader with diffuse texture (no lighting)
 * 
 * @var ShaderKind::ShadelessTexNone
 *      Unlit shader without texture (flat color)
 * 
 * @var ShaderKind::Comp
 *      Compute shader placeholder
 * 
 * @var ShaderKind::Quad
 *      Full-screen quad shader for post-processing
 * 
 * @var ShaderKind::Bloom
 *      Bloom effect post-processing shader
 * 
 * @var ShaderKind::DebugNormals
 *      Normal visualization debug shader
 * 
 * @var ShaderKind::ShadowMap
 *      Shadow map generation shader
 * 
 * @var ShaderKind::Skybox
 *      Skybox/cubemap rendering shader
 */
enum class ShaderKind {
	None = 0,			///< No shader
	Debug,				///< Debug visualization
	LightTexNone,		///< Light + no texture
	LightTexD,			///< Light + diffuse texture
	LightTexDN,			///< Light + diffuse + normal map
	LightTexDNS,		///< Light + diffuse + normal + specular
	Occlusion,			///< Occlusion query
	ShadelessTexD,		///< Unlit + diffuse texture
	ShadelessTexNone,	///< Unlit, flat color
	Comp,				///< Compute shader
	Quad,				///< Screen quad
	Bloom,				///< Bloom post-process
	DebugNormals,		///< Normal visualization
	ShadowMap,			///< Shadow map generation
	Skybox				///< Skybox rendering
};
