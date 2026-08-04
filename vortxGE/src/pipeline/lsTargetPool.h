/**
 * @file lsTargetPool.h
 * @brief Named render targets with a single resize owner (Plan.md Part 3).
 *
 * Replaces ad-hoc lsFBO_* datablock wiring: stages refer to targets by
 * NAME ("scene", "shadow0", ...), the pool creates/resizes the real
 * FBOs when the app reports ONE framebuffer size. Target 0 is always
 * the screen (default framebuffer).
 */
#pragma once

#include <string>
#include <unordered_map>

#include "../render/lsRender.h"

class lsTargetPool
{
public:
	static constexpr lsFBOID SCREEN = 0;	// default framebuffer

	explicit lsTargetPool( lsRender* render ) : m_render( render ) {}
	~lsTargetPool();

	/// Declare (or fetch) a colour target; depth=true adds a depth
	/// attachment. scale=1.0 tracks the framebuffer size.
	lsFBOID acquire( const std::string& name, bool withDepth = true,
					 float scale = 1.0f );
	lsFBOID find( const std::string& name ) const;	// 0 when absent
	lsTextureID colorTexture( const std::string& name ) const;

	/// Single resize owner: app reports the framebuffer pixel size
	/// once; every named target is (re)created to match (× scale).
	void setFramebufferSize( uint32_t w, uint32_t h );
	uint32_t width() const { return m_w; }
	uint32_t height() const { return m_h; }

private:
	struct Entry
	{
		lsFBOID		fbo = 0;
		lsTextureID	color = 0;
		lsTextureID	depthTex = 0;
		bool		withDepth = true;
		float		scale = 1.0f;
		bool		realized = false;
	};
	void realize( Entry& e );
	void destroy( Entry& e );

	lsRender*	m_render;
	std::unordered_map<std::string, Entry> m_entries;
	uint32_t	m_w = 0, m_h = 0;
};
