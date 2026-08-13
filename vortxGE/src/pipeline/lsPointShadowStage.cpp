#include "lsPointShadowStage.h"

#include "../datablocks/lsFBO_DepthCube.h"
#include "../scene/lsCamera.h"
#include "../scene/lsLight.h"
#include "../scene/lsScene.h"
#include "lsStagePassToken.h"

#if defined(VORTX_DEBUG_TOOLS)
#include <cstdlib>
#include <iostream>
#include <vector>
#include <GL/glew.h>
#include "../render/lsRender.h"
static bool pointShadowTrace()
{
	static const bool v = std::getenv( "VORTX_SHADOW_TRACE" ) != nullptr;
	return v;
}
static bool pointShadowDebug()
{
	static const bool v = std::getenv( "VORTX_POINTSHADOW_DEBUG" ) != nullptr;
	return v;
}
#endif

// GL cube-face orientation convention: face i samples direction
// (dir), rendered with lookAt(lightPos, lightPos+dir, up)
static const lsVector3 FACE_DIR[6] = {
	lsVector3(  1,  0,  0 ), lsVector3( -1,  0,  0 ),
	lsVector3(  0,  1,  0 ), lsVector3(  0, -1,  0 ),
	lsVector3(  0,  0,  1 ), lsVector3(  0,  0, -1 ) };
static const lsVector3 FACE_UP[6] = {
	lsVector3(  0, -1,  0 ), lsVector3(  0, -1,  0 ),
	lsVector3(  0,  0,  1 ), lsVector3(  0,  0, -1 ),
	lsVector3(  0, -1,  0 ), lsVector3(  0, -1,  0 ) };

lsPointShadowStage::lsPointShadowStage( lsScene* scene, lsLight* light,
										int mapSize, lsRender* render )
	: m_scene( scene ), m_light( light )
{
	m_cube = new lsFBO_DepthCube( mapSize, render );
	if( m_cube && m_light )
	{
		// self-wire (same convention as lsShadowMap): shading finds the
		// cube texture through the light
		m_light->setTex_shadowmap( m_cube->getDepthTexture() );
		m_light->setHas_shadowmap( true );
		m_light->setPixel_x( mapSize );
		m_light->setPixel_y( mapSize );
	}
#if defined(VORTX_DEBUG_TOOLS)
	if( pointShadowTrace() )
		std::cout << "POINTSHADOWSTAGE ctor light="
				  << ( light ? light->getName() : "(null)" )
				  << " size=" << mapSize << std::endl;
#endif
}

lsPointShadowStage::~lsPointShadowStage()
{
	delete m_passToken;
	delete m_cube;
}

void lsPointShadowStage::execute( lsRender* render, lsTargetPool& targets )
{
	if( m_light && !m_light->cast_shadows ) return; // L3.3
	(void)targets;
	if( !m_scene || !m_light || !m_cube ) return;

	if( !m_passToken )
		m_passToken = new lsStagePassToken( render, LSPT_SHADOWMAP );

	// light world position
	const lsVector3 lightPos( m_light->ModelMatrix.matrix[12],
							  m_light->ModelMatrix.matrix[13],
							  m_light->ModelMatrix.matrix[14] );
	const float nearZ = m_light->point_shadow_near;
	const float farZ  = m_light->point_shadow_far;

	render->Enable( LS_DEPTH_TEST );
	render->DepthRangef( 0.0f, 1.0f );
	render->Enable( LS_POLYGON_OFFSET_FILL );
	render->PolygonOffset( 2.0f, 5.0f );

	const int size = m_cube->size();
	for( int face = 0; face < 6; ++face )
	{
		// throwaway cull/render camera for this face (lsCsmStage pattern)
		lsCamera cam( nullptr );
		cam.ViewMatrix.LookAt( lightPos, lightPos + FACE_DIR[face],
							   FACE_UP[face] );
		cam.frustum.Set_Perspective( 90.0f, 1.0f, nearZ, farZ );

		m_scene->InitDrawlist();
		m_scene->Optimize( &cam, m_passToken, m_scene, render );
		m_scene->OptimizeThread( &cam, m_passToken, m_scene, render );

		m_cube->EnableFace( face );
		render->Viewport( 0, 0, size, size );
		m_cube->ClearBuffers();

		m_scene->Draw( &cam, m_passToken, nullptr, render );
	}

	// restore defaults for the next stage
	render->BindFramebuffer( LS_FRAMEBUFFER, lsTargetPool::SCREEN );
	render->Disable( LS_POLYGON_OFFSET_FILL );
	render->CullFace( LS_BACK );

#if defined(VORTX_DEBUG_TOOLS)
	// VORTX_POINTSHADOW_DEBUG=1: one-shot depth probe of the -Y (floor)
	// face at frame 30 — min<1 means the cube has occluder content
	static int dbgFrame = 0;
	if( pointShadowDebug() && render->getType() == RenderType::OpenGL &&
		++dbgFrame == 30 )
	{
		m_cube->EnableFace( 3 );
		const int size = m_cube->size();
		std::vector<float> depth( (size_t)size * size );
		glReadPixels( 0, 0, size, size, GL_DEPTH_COMPONENT, GL_FLOAT,
					  depth.data() );
		float mn = 1e30f, mx = -1e30f;
		for( float d : depth ) { if( d < mn ) mn = d; if( d > mx ) mx = d; }
		std::cout << "POINTDBG face -Y depth min=" << mn << " max=" << mx
				  << std::endl;
		render->BindFramebuffer( LS_FRAMEBUFFER, lsTargetPool::SCREEN );
	}
#endif
}
