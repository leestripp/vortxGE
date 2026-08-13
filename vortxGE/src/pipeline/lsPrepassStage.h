/**
 * @file lsPrepassStage.h
 * @brief L4.1b depth+normal prepass stage (Plan.md Part 4 L4)
 *
 * Renders the scene from the main camera into the "prepass" pool
 * target (colour = view-space normals, depth = scene depth) BEFORE
 * the main scene pass. Enables L4.2 screen-space contact shadows
 * now and SSAO/SSR later without a deferred rewrite.
 */

#pragma once

#include <string>

#include "lsRenderStage.h"
#include "lsTargetPool.h"
#include "../render/lsRenderTypes.h"

class lsScene;
class lsCamera;
class lsStagePassToken;

class lsPrepassStage : public lsRenderStage
{
public:
	lsPrepassStage( lsScene* scene, lsCamera* camera,
					const std::string& targetName = "prepass" );
	~lsPrepassStage() override;

	Type type() const override { return Type::Prepass; }
	void execute( lsRender* render, lsTargetPool& targets ) override;

	/// Depth texture of the prepass target after execute (0 if none).
	/// Read by lsShader to bind L4.2 contact-shadow ray marching.
	static lsTextureID depthTexture() { return s_depthTex; }

private:
	lsScene* m_scene = nullptr;
	lsCamera* m_camera = nullptr;
	std::string m_target;
	lsStagePassToken* m_passToken = nullptr;

	static lsTextureID s_depthTex;
};
