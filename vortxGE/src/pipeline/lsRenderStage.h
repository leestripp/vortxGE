/**
 * @file lsRenderStage.h
 * @brief Base class for render-pipeline stages (Plan.md Part 3).
 *
 * A stage is one explicit, self-contained rendering step: it declares
 * its inputs/outputs up front and sets ALL render state it needs, so
 * nothing leaks between stages (unlike the old lsPass linked list).
 * Subclasses: lsSceneStage (scene+camera→target), lsBlitStage
 * (target→target/screen fullscreen blit — replaces the Quad-Viewer
 * control hack), lsShadowStage, lsEffectStage (future post-fx).
 */
#pragma once

#include <string>

#include "../render/lsRender.h"

class lsTargetPool;

class lsRenderStage
{
public:
	enum class Type { Scene, Shadow, Blit, Effect, Prepass };

	virtual ~lsRenderStage() = default;

	virtual Type type() const = 0;

	/// Run the stage. `targets` resolves named inputs/outputs;
	/// SCREEN (target 0) is the default framebuffer.
	virtual void execute( lsRender* render, lsTargetPool& targets ) = 0;

	/// Viewport rect in NORMALIZED framebuffer coords (0..1). The
	/// pipeline — not the stage — turns this into a pixel Viewport
	/// call (single HiDPI/resize owner, Plan.md Part 3 rule 5).
	void setRect( float x, float y, float w, float h )
	{ m_rect[0]=x; m_rect[1]=y; m_rect[2]=w; m_rect[3]=h; }
	const float* rect() const { return m_rect; }

	void setName( const std::string& name ) { m_name = name; }
	const std::string& name() const { return m_name; }

	bool isEnabled() const { return m_enabled; }
	void setEnabled( bool v ) { m_enabled = v; }

protected:
	float		m_rect[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	std::string m_name;
	bool		m_enabled = true;
};
