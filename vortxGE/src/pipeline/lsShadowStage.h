/**
 * @file lsShadowStage.h
 * @brief Renders a light's shadow (depth) map (Plan.md Part 3, R4).
 *
 * Replaces lsPass_ShadowMap: owns an lsFBO_Depth, renders the scene
 * from the light's perspective (lights ARE cameras in this engine)
 * with polygon-offset depth bias, and self-wires the depth texture
 * back into the light for the PBR shaders.
 */
#pragma once

#include "lsRenderStage.h"
#include "lsTargetPool.h"

class lsScene;
class lsLight;
class lsFBO_Depth;
class lsPass;

class lsShadowStage : public lsRenderStage
{
public:
	/// Creates the depth FBO immediately (mirrors lsPass_ShadowMap).
	/// size: square map resolution (demo uses 4096 direct / 2048 spot).
	lsShadowStage( lsScene* scene, lsLight* light, int size,
				   lsRender* render );
	~lsShadowStage() override;

	Type type() const override { return Type::Shadow; }
	void execute( lsRender* render, lsTargetPool& targets ) override;

	lsFBO_Depth* fbo() const { return m_fbo; }

private:
	lsScene*	m_scene;
	lsLight*	m_light;
	lsFBO_Depth* m_fbo = nullptr;
	int			m_size;
	lsPass*		m_passToken = nullptr;
};
