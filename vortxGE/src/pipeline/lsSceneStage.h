/**
 * @file lsSceneStage.h
 * @brief Renders an lsScene with an explicit lsCamera into a named
 *        render target (Plan.md Part 3, Phase R2).
 *
 * Replaces lsPass_FBO + lsPass_DR scene rendering. Camera is a plain
 * pointer — no cull_camera dual-wiring, no control chain. All render
 * state the stage needs is set inside execute(); nothing leaks.
 */
#pragma once

#include "lsRenderStage.h"
#include "lsTargetPool.h"

class lsScene;
class lsCamera;
class lsPass;

class lsSceneStage : public lsRenderStage
{
public:
	/// targetName: pool name ("scene", ...) or "" / "screen" for the
	/// default framebuffer.
	lsSceneStage( lsScene* scene, lsCamera* camera,
				  const std::string& targetName );
	~lsSceneStage() override;

	Type type() const override { return Type::Scene; }
	void execute( lsRender* render, lsTargetPool& targets ) override;

	void setCamera( lsCamera* cam ) { m_camera = cam; }
	lsCamera* camera() const { return m_camera; }
	void setScene( lsScene* scn ) { m_scene = scn; }
	void setTarget( const std::string& t ) { m_target = t; }

	void setClearColor( float r, float g, float b, float a )
	{ m_clear[0]=r; m_clear[1]=g; m_clear[2]=b; m_clear[3]=a; }

private:
	lsScene*	m_scene;
	lsCamera*	m_camera;
	std::string m_target;
	float		m_clear[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	// Legacy bridge: the scene object Optimize/Draw path requires a
	// non-null lsPass with a pass_type (LSPT_FBO selects the standard
	// material shaders). Owned minimal token; removed in Phase R6 when
	// the object path stops taking lsPass.
	lsPass*		m_passToken = nullptr;
};
