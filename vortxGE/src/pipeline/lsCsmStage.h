/**
 * @file lsCsmStage.h
 * @brief Renders cascaded sun shadow maps into the atlas (Plan.md L2.1b).
 *
 * Per frame: recomputes cascade fits from the render camera (via
 * lsCascadedShadowMap::update), then renders the scene once per
 * cascade into the cascade's atlas quadrant, using a throwaway
 * lsCamera carrying the cascade's view/frustum (culling + shadow
 * depth shader work unchanged).
 */
#pragma once

#include "lsRenderStage.h"
#include "lsCascadedShadowMap.h"
#include "lsTargetPool.h"

class lsScene;
class lsLight;
class lsCamera;

class lsCsmStage : public lsRenderStage
{
public:
	/// scene: geometry source; light: the sun (directional);
	/// camera: render camera the cascades are fitted to;
	/// mapSize: per-cascade resolution (atlas is 2x).
	lsCsmStage( lsScene* scene, lsLight* light, lsCamera* camera,
				int mapSize, lsRender* render );
	~lsCsmStage() override;

	Type type() const override { return Type::Shadow; }
	void execute( lsRender* render, lsTargetPool& targets ) override;

	lsCascadedShadowMap* csm() const { return m_csm; }

private:
	lsScene*				m_scene;
	lsLight*				m_light;
	lsCamera*				m_camera;
	lsCascadedShadowMap*	m_csm = nullptr;
	int					m_passType = LSPT_SHADOWMAP;	// R6: plain pass-type int
};
