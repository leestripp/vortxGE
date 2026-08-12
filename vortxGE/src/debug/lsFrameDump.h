/**
 * @file lsFrameDump.h
 * @brief D2 (Plan.md): per-draw frame state dump ("mini-RenderDoc").
 *
 * `VORTX_FRAME_DUMP=N:path.json` records every draw call of frame N
 * (default 30) with object/program/VAO identity, UBO contents
 * (matrices + light arrays), texture/unit bindings and pipeline
 * state, and writes the result as JSON. Pair one GL dump with one VK
 * dump and compare them with `tools/diff_frames.py`.
 *
 * Like all D-series tooling this header compiles to NOTHING when
 * VORTX_DEBUG_TOOLS is undefined (Release): no symbols, no strings,
 * no branches. All call sites are wrapped in VORTX_DEBUG_ONLY().
 *
 * Data sources are deliberately asymmetric so backend-internal bugs
 * are visible in the diff:
 *  - GL records state TRACKED at the lsRender API boundary (what the
 *    engine asked for).
 *  - Vulkan snapshots its INTERNAL structures at draw time (uboStaging
 *    at std140 member offsets, sampler-unit maps, texture units) —
 *    i.e. what the GPU will actually read. A std140 offset or
 *    staging bug therefore shows up as a uniform mismatch even though
 *    both backends received identical Uniform* calls.
 *
 * Cross-backend identity (driver-assigned GL ids vs sequential VK
 * ids) is carried by signatures, not raw ids: programs by FNV-1a
 * hash of shader sources, textures by (w, h, internalformat, content
 * hash). diff_frames.py canonicalises the rest (FBO/VAO ordinals).
 */
#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#if defined(VORTX_DEBUG_TOOLS)

namespace lsFrameDump
{
	/// One recorded draw call.
	struct DrawRec
	{
		uint32_t	fbo = 0;			// render target (0 = default/swapchain)
		uint32_t	prog = 0, vao = 0;
		uint32_t	mode = 0;			// LS_* primitive mode (GL numerics)
		uint32_t	count = 0, first = 0;	// first = firstIndex (indexed) / first vertex
		bool		indexed = true;
		int			vp[4] = { 0, 0, 0, 0 };	// viewport x, y, w, h
		// pipeline state (GL-convention values, both backends)
		bool		depthTest = false, depthWrite = true;
		uint32_t	depthFunc = 0;
		bool		cull = false;
		uint32_t	cullFace = 0;
		bool		blend = false;
		uint32_t	blendSrc = 0, blendDst = 0;
		std::map<uint32_t, uint32_t>			tex;	// unit -> texture id
		std::map<std::string, std::vector<float>>	u;	// float uniforms
		std::map<std::string, std::vector<int>>		iu;	// int uniforms + sampler->unit
	};

	/// Backend self-identification ("opengl" / "vulkan"); call once.
	void setBackend( const char* name );
	/// True while the target frame is being rendered (cheap).
	bool recording();
	/// Begin a draw record; nullptr when not recording. Used by the
	/// Vulkan backend, which fills the record from its own internals.
	DrawRec* openDraw();
	/// Finish the record opened by openDraw().
	void commitDraw();
	/// End-of-frame tick — call once per presented frame from Present().
	/// Writes the JSON file when the target frame completes.
	void frameTick();

	/// Texture identity hook — both backends call from TexImage2D.
	/// @p tex is the engine-level texture id being defined.
	void trackTexImage( uint32_t tex, int level, int internalFormat,
						int w, int h, int format, int type, const void* data );
	/// GL variant: resolves the texture from tracked unit/binding state.
	void trackTexImageBound( int level, int internalFormat,
						 int w, int h, int format, int type, const void* data );

	/// Program identity hooks — both backends.
	void trackShaderSource( uint32_t shader, const std::string& src );
	void trackAttach( uint32_t program, uint32_t shader );
	void trackLink( uint32_t program );

	/// GL-side state tracking (lsOpenGL.cpp only — VK reads its own).
	void trackUniformName( uint32_t program, int location, const char* name );
	void trackUseProgram( uint32_t program );
	void trackBindVAO( uint32_t vao );
	void trackBindFBO( uint32_t fbo );
	void trackViewport( int x, int y, int w, int h );
	void trackActiveTexture( uint32_t unit );
	void trackBindTexture( uint32_t tex );
	void trackUniformF( int location, const float* v, int n );
	void trackUniformI( int location, const int* v, int n );
	void trackEnable( uint32_t cap, bool on );
	void trackCullFace( uint32_t mode );
	void trackBlendFunc( uint32_t src, uint32_t dst );
	void trackDepthFunc( uint32_t func );
	void trackDepthMask( bool on );
	/// Record a draw from the tracked GL state (lsOpenGL.cpp only).
	void glRecordDraw( uint32_t mode, uint32_t count, uint32_t first,
					   bool indexed );
}

#endif // VORTX_DEBUG_TOOLS
