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
	/// Vulkan's Y-flipped viewport inverts FBO texel rows vs GL; enable
	/// to flip V when sampling (backend-agnostic stages need the app to
	/// say which convention the backend uses).
	void setTint( float r, float g, float b, float a )
	{ m_tint[0]=r; m_tint[1]=g; m_tint[2]=b; m_tint[3]=a; }

	/// Vertical flip of the sampled target. -1 = auto (flips when the
	/// backend renders FBO textures with a Y-flipped viewport — Vulkan's
	/// VK_KHR_maintenance1 convention — so the blit matches GL output),
	/// 0/1 = force off/on. Applies at first execute (lazy init).
	void setFlipY( int mode ) { m_flipY = mode; }

	/// V-flip the sampled image. Needed on Vulkan: the backend uses a
	/// negative-height (GL-compatible) viewport, so FBO texels are
	/// stored V-mirrored vs OpenGL — "offscreen rendering must be
	/// compensated" (VK_KHR_maintenance1 viewport-flip convention).
	/// X is NOT affected (the viewport flip only negates height).

private:
	bool lazyInit( lsRender* render );

	std::string m_src, m_dst;
	float		m_tint[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	int			m_flipY = -1;	// -1 auto, 0 off, 1 on

	// lazy GL objects
	lsProgramID	m_prog = 0;
	lsVAOID		m_vao = 0;
	lsBufferID	m_vbo = 0;
	lsUniformID	m_uSampler = -1;
	lsUniformID	m_uTint = -1;
	bool		m_ready = false;
};
