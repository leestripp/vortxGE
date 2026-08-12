/**
 * @file lsCsm.h
 * @brief Cascaded shadow map math (Plan.md Part 4, L2.1/L2.2).
 *
 * Pure math, no engine/render dependencies (doctest-friendly):
 * practical split scheme + per-cascade stable ortho fitting to camera
 * frustum slice bounds with texel snapping. The pipeline-side owner
 * (lsCascadedShadowMap, L2.1b) calls this once per frame and feeds
 * the results to the shadow stage + shaders.
 */
#pragma once

#include <vector>

#include "lsMatrix4x4.h"
#include "lsVector3.h"

/// CSM tuning parameters
struct lsCsmConfig
{
	int		cascades = 4;		///< 1..4 (atlas is 2x2)
	float	lambda = 0.5f;		///< split blend: 0=uniform, 1=log
	int		mapSize = 2048;		///< per-cascade resolution (texels)
	float	maxDistance = 0.0f;	///< shadow range cap; 0 = camera far
};

/// One fitted cascade
struct lsCsmCascade
{
	lsMatrix4x4	view;			///< world -> light space
	lsMatrix4x4	proj;			///< fitted ortho (GL NDC convention)
	lsMatrix4x4	shadowMatrix;	///< bias * proj * view (texture space)
	float		splitNear = 0.0f;	///< view-space depth range covered
	float		splitFar = 0.0f;
};

/// Practical split scheme: lambda blend of logarithmic and uniform.
/// Returns cascades+1 depths; [0]=nearZ, [cascades]=farZ.
std::vector<float> lsCsmSplitDepths( float nearZ, float farZ,
									 int cascades, float lambda );

/// Compute per-cascade matrices fitted to the camera frustum slices.
///
/// Stability (L2.2): each cascade fits a bounding SPHERE around its
/// slice (rotation-invariant) and texel-snaps the light-space centre
/// to the cascade's texel grid — no swimming while moving/rotating.
///
/// @param cfg       cascade count/lambda/mapSize/maxDistance
/// @param camView   world -> view matrix of the render camera
/// @param fovYDeg   vertical field of view (degrees)
/// @param aspect    width / height
/// @param nearZ/farZ camera clip planes
/// @param lightDir  world-space direction the light travels (sun dir)
/// @param yFlip     Vulkan offscreen-FBO V-mirror compensation
///                  (same convention as lsLight::buildShadowmap_matrix)
std::vector<lsCsmCascade> lsCsmCompute( const lsCsmConfig& cfg,
										const lsMatrix4x4& camView,
										float fovYDeg, float aspect,
										float nearZ, float farZ,
										const lsVector3& lightDir,
										bool yFlip );
