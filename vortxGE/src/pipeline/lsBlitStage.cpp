#include "lsBlitStage.h"

#include <iostream>

using namespace std;

namespace
{
const char* BLIT_VS = R"(#version 410 core
layout(location = 0) in vec2 vxPos;
layout(location = 1) in vec2 vxUV;
out vec2 ioUV;
void main()
{
	ioUV = vxUV;
	gl_Position = vec4( vxPos, 0.0, 1.0 );
}
)";

const char* BLIT_FS = R"(#version 410 core
uniform sampler2D tex_source;
uniform vec4 vxTint;
in vec2 ioUV;
out vec4 fragColour;

// linear -> sRGB transfer (matches demos/data/shaders/quad.frag so the
// pipeline blit is visually identical to the legacy Quad-Viewer path)
vec3 toSRGB( vec3 rgb )
{
	return mix( 1.055 * pow( rgb, vec3( 1.0 / 2.4 ) ) - 0.055,
				rgb * 12.92,
				lessThanEqual( rgb, vec3( 0.0031308 ) ) );
}

void main()
{
	vec4 c = texture( tex_source, ioUV ) * vxTint;
	fragColour = vec4( toSRGB( clamp( c.rgb, 0.0, 1.0 ) ), c.a );
}
)";

// fullscreen quad: pos.xy, uv.xy x 6 vertices (2 triangles)
const float QUAD[] = {
	-1.0f, -1.0f,  0.0f, 0.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	 1.0f,  1.0f,  1.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	 1.0f,  1.0f,  1.0f, 1.0f,
	-1.0f,  1.0f,  0.0f, 1.0f,
};
// V-flipped variant (uv.y = 1 - uv.y) for Vulkan FBO compensation
const float QUAD_FLIPY[] = {
	-1.0f, -1.0f,  0.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 1.0f,
	 1.0f,  1.0f,  1.0f, 0.0f,
	-1.0f, -1.0f,  0.0f, 1.0f,
	 1.0f,  1.0f,  1.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 0.0f,
};
} // namespace

lsBlitStage::lsBlitStage( const std::string& src, const std::string& dst )
	: m_src( src ), m_dst( dst )
{
}

lsBlitStage::~lsBlitStage()
{
	// GL objects are released with the render context; the engine
	// shutdown owns teardown order.
}

bool lsBlitStage::lazyInit( lsRender* render )
{
	if( m_ready ) return true;

	// auto flipY: Vulkan renders FBO textures Y-flipped vs GL
	// (negative-height viewport), so sampling needs V flipped to match
	if( m_flipY < 0 )
		m_flipY = ( render->getType() == RenderType::Vulkan ) ? 1 : 0;

	lsShaderID vs = render->CreateShader( LS_VERTEX_SHADER );
	render->ShaderSource( vs, 1, &BLIT_VS, nullptr );
	render->CompileShader( vs );

	lsShaderID fs = render->CreateShader( LS_FRAGMENT_SHADER );
	render->ShaderSource( fs, 1, &BLIT_FS, nullptr );
	render->CompileShader( fs );

	m_prog = render->CreateProgram();
	render->AttachShader( m_prog, vs );
	render->AttachShader( m_prog, fs );
	render->LinkProgram( m_prog );
	render->DeleteShader( vs );
	render->DeleteShader( fs );

	lsInt linked = 0;
	render->GetProgramiv( m_prog, LS_LINK_STATUS, &linked );
	if( !linked )
	{
		char log[1024]{};
		render->GetProgramInfoLog( m_prog, sizeof( log ), nullptr, log );
		cerr << "lsBlitStage: shader link failed: " << log << endl;
		return false;
	}

	m_uSampler = render->GetUniformLocation( m_prog, "tex_source" );
	m_uTint = render->GetUniformLocation( m_prog, "vxTint" );

	render->GenVertexArrays( 1, &m_vao );
	render->BindVertexArray( m_vao );
	render->GenBuffers( 1, &m_vbo );
	render->BindBuffer( LS_ARRAY_BUFFER, m_vbo );
	const float* quadData = m_flipY ? QUAD_FLIPY : QUAD;
	render->BufferData( LS_ARRAY_BUFFER, sizeof( QUAD ), quadData,
						LS_STATIC_DRAW );
	render->EnableVertexAttribArray( 0 );
	render->VertexAttribPointer( 0, 2, LS_FLOAT, LS_FALSE, 4 * sizeof( float ),
								 (const void*)0 );
	render->EnableVertexAttribArray( 1 );
	render->VertexAttribPointer( 1, 2, LS_FLOAT, LS_FALSE, 4 * sizeof( float ),
								 (const void*)( 2 * sizeof( float ) ) );
	render->BindVertexArray( 0 );

	m_ready = true;
	return true;
}

void lsBlitStage::execute( lsRender* render, lsTargetPool& targets )
{
	const lsTextureID tex = targets.colorTexture( m_src );
	if( !tex || !lazyInit( render ) ) return;

	const bool toScreen = ( m_dst.empty() || m_dst == "screen" );
	const lsFBOID fbo = toScreen ? lsTargetPool::SCREEN
								 : targets.find( m_dst );

	render->BindFramebuffer( LS_FRAMEBUFFER, fbo );

	const float tw = (float)targets.width();
	const float th = (float)targets.height();
	render->Viewport( (lsInt)( m_rect[0] * tw ), (lsInt)( m_rect[1] * th ),
					  (lsSizei)( m_rect[2] * tw ), (lsSizei)( m_rect[3] * th ) );

	// blit state: no depth, no culling (winding-immune), no blending
	render->Disable( LS_DEPTH_TEST );
	render->Disable( LS_CULL_FACE );
	render->DepthMask( LS_FALSE );
	render->Disable( LS_BLEND );

	render->UseProgram( m_prog );
	render->Uniform1i( m_uSampler, 0 );
	render->Uniform4f( m_uTint, m_tint[0], m_tint[1], m_tint[2], m_tint[3] );
	render->ActiveTexture( LS_TEXTURE0 );
	render->BindTexture( LS_TEXTURE_2D, tex );

	render->BindVertexArray( m_vao );
	render->DrawArrays( LS_TRIANGLES, 0, 6 );
	render->BindVertexArray( 0 );

	// restore common defaults for the next stage
	render->DepthMask( LS_TRUE );
	render->Enable( LS_DEPTH_TEST );
}
