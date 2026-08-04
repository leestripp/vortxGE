/**
 * @file lsBlitStage.h
 * @brief Fullscreen blit: named target -> named target/screen
 *        (Plan.md Part 3, Phase R2).
 *
 * Replaces the entire "Quad Viewer" control hack (hidden lsScene +
 * ortho camera + 180-degree-rotated quad object + lsPass_DR) with one
 * self-contained stage: a tiny built-in shader, a 2-triangle VBO,
 * depth/cull disabled. No scene, no camera, no winding hazards.
 */
#pragma once

#include "lsRenderStage.h"
#include "lsTargetPool.h"

class lsBlitStage : public lsRenderStage
{
public:
	/// src: pool target name to sample ("scene", ...).
	/// dst: pool target name, or "" / "screen" for the default fb.
	lsBlitStage( const std::string& src, const std::string& dst = "" );
	~lsBlitStage() override;

	Type type() const override { return Type::Blit; }
	void execute( lsRender* render, lsTargetPool& targets ) override;

	void setSource( const std::string& src ) { m_src = src; }
	void setDest( const std::string& dst ) { m_dst = dst; }
	void setTint( float r, float g, float b, float a )
	{ m_tint[0]=r; m_tint[1]=g; m_tint[2]=b; m_tint[3]=a; }

private:
	bool lazyInit( lsRender* render );

	std::string m_src, m_dst;
	float		m_tint[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	// lazy GL objects
	lsProgramID	m_prog = 0;
	lsVAOID		m_vao = 0;
	lsBufferID	m_vbo = 0;
	lsUniformID	m_uSampler = -1;
	lsUniformID	m_uTint = -1;
	bool		m_ready = false;
};
