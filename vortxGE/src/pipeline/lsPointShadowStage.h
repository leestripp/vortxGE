/**
 * @file lsPointShadowStage.h
 * @brief Pipeline stage: omnidirectional (point light) shadow cube
 *
 * L3.2 (Plan.md). Renders the scene six times from the light position
 * into the faces of an lsFBO_DepthCube (90° perspective per face,
 * GL cube-face orientation convention). Face projections use the
 * light's point_shadow_near/far; shading (use_shadowmap=3) samples
 * the cube as samplerCubeShadow and reconstructs the reference depth
 * from the fragment's major-axis distance through the same projection.
 */
#pragma once

#include "lsRenderStage.h"
#include "lsTargetPool.h"

class lsScene;
class lsLight;
class lsRender;
class lsFBO_DepthCube;
class lsStagePassToken;

class lsPointShadowStage : public lsRenderStage
{
public:
	lsPointShadowStage( lsScene* scene, lsLight* light, int mapSize,
						lsRender* render );
	~lsPointShadowStage() override;

	Type type() const override { return Type::Shadow; }
	void execute( lsRender* render, lsTargetPool& targets ) override;

	lsFBO_DepthCube* cube() const { return m_cube; }

private:
	lsScene*			m_scene = nullptr;
	lsLight*			m_light = nullptr;
	lsFBO_DepthCube*	m_cube = nullptr;
	lsStagePassToken*	m_passToken = nullptr;
};
