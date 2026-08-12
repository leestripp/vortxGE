/**
 * @file lsShadowStage.h
 * @brief Renders a light's shadow (depth) map (Plan.md Part 3, R4).
 *
 * L1.6: thin driver over lsShadowMap (L1.1) — the subsystem owns the
 * depth target, resolution policy and light wiring; this stage just
 * culls and renders the scene from the light's perspective each frame
 * with polygon-offset depth bias.
 */
#pragma once

#include "lsRenderStage.h"
#include "lsShadowMap.h"
#include "lsTargetPool.h"

class lsScene;
class lsLight;
class lsFBO_Depth;
class lsPass;

class lsShadowStage : public lsRenderStage
{
public:
	/// Creates the shadow map immediately via the subsystem.
	/// quality: resolution preset (High = 4096 direct / 2048 spot).
	lsShadowStage( lsScene* scene, lsLight* light,
				   lsShadowQuality quality, lsRender* render );
	~lsShadowStage() override;

	Type type() const override { return Type::Shadow; }
	void execute( lsRender* render, lsTargetPool& targets ) override;

	lsFBO_Depth* fbo() const { return m_shadowMap ? m_shadowMap->fbo() : nullptr; }

private:
	lsScene*		m_scene;
	lsLight*		m_light;
	lsShadowMap*	m_shadowMap = nullptr;
	lsPass*			m_passToken = nullptr;
};
