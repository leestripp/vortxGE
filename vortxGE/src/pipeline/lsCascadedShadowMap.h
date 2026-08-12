/**
 * @file lsCascadedShadowMap.h
 * @brief Cascaded shadow maps for directional lights (Plan.md L2.1b/2.3).
 *
 * Owns ONE depth atlas (2x2 quadrants, one lsFBO_Depth) and recomputes
 * the per-cascade light matrices each frame via lsCsm (L2.1a). The
 * CSM stage (lsCsmStage) renders each cascade into its quadrant.
 */
#pragma once

#include <vector>

#include "../math/lsCsm.h"

class lsFBO_Depth;
class lsLight;
class lsCamera;
class lsRender;

class lsCascadedShadowMap
{
public:
	/// Creates the atlas immediately (2*mapSize square, up to 4
	/// quadrants). mapSize = per-cascade resolution from the quality
	/// preset (High: 2048/cascade -> 4096 atlas, matching the legacy
	/// single-map sun footprint).
	lsCascadedShadowMap( lsLight* light, int mapSize, lsRender* render );
	~lsCascadedShadowMap();

	/// Recompute cascade matrices for this frame. cam: render camera;
	/// lightDir: world-space sun direction; yFlip: VK offscreen V-mirror.
	void update( const lsMatrix4x4& camView, float fovYDeg, float aspect,
				 float nearZ, float farZ, const lsVector3& lightDir,
				 bool yFlip );

	const std::vector<lsCsmCascade>& cascades() const { return m_cascades; }

	lsFBO_Depth*	atlas() const { return m_atlas; }
	int				mapSize() const { return m_mapSize; }

	/// Atlas quadrant rect for cascade i (pixels)
	void quadrant( int i, int& x, int& y ) const
	{
		x = ( i & 1 ) * m_mapSize;
		y = ( i >> 1 ) * m_mapSize;
	}

	lsCsmConfig&	config() { return m_config; }

private:
	lsLight*					m_light = nullptr;
	lsFBO_Depth*				m_atlas = nullptr;
	int							m_mapSize = 0;
	lsCsmConfig					m_config;
	std::vector<lsCsmCascade>	m_cascades;
};
