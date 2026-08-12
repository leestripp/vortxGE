#include "lsShadowStage.h"

#include "../datablocks/lsFBO_Depth.h"
#include "../scene/lsLight.h"
#include "../scene/lsScene.h"
#include "lsStagePassToken.h"

#if defined(VORTX_DEBUG_TOOLS)
#include <cstdlib>
#include <iostream>
// VORTX_SHADOW_TRACE=1: log stage construction + per-frame early-out
// reasons (first frames only) — catches type/gating mismatches between
// shadow rendering and shading.
static bool shadowTrace()
{
	static const bool v = std::getenv( "VORTX_SHADOW_TRACE" ) != nullptr;
	return v;
}
#endif

lsShadowStage::lsShadowStage( lsScene* scene, lsLight* light, int size,
							  lsRender* render )
	: m_scene( scene ), m_light( light ), m_size( size )
{
#if defined(VORTX_DEBUG_TOOLS)
	if( shadowTrace() )
		std::cout << "SHADOWSTAGE ctor light=" << ( light ? light->getName() : "(null)" )
				  << " type=" << ( light ? (int)light->getLight_type() : -1 )
				  << " size=" << size << std::endl;
#endif
	if( !light || size <= 0 ) return;

	m_fbo = new lsFBO_Depth( (double)size, (double)size, render );

	// self-wire: hand the depth texture to the light for PBR sampling
	// (mirrors DemoScene::setupShadows)
	if( m_fbo )
	{
		light->setTex_shadowmap( m_fbo->getDepthTexture() );
		light->setHas_shadowmap( true );
		light->setPixel_x( size );	// L1.5: texel snapping needs the size
		light->setPixel_y( size );
	}
}

lsShadowStage::~lsShadowStage()
{
	delete m_passToken;
	delete m_fbo;
}

void lsShadowStage::execute( lsRender* render, lsTargetPool& targets )
{
	(void)targets;
#if defined(VORTX_DEBUG_TOOLS)
	static uint32_t traceLeft = 12;
	if( shadowTrace() && traceLeft &&
		( !m_scene || !m_light || !m_fbo || !m_light->getActive() ) )
	{
		traceLeft--;
		std::cout << "SHADOWSTAGE skip light="
				  << ( m_light ? m_light->getName() : "(null)" )
				  << " scene=" << ( m_scene != nullptr )
				  << " fbo=" << ( m_fbo != nullptr )
				  << " active=" << ( m_light ? m_light->getActive() : false )
				  << std::endl;
	}
#endif
	if( !m_scene || !m_light || !m_fbo ) return;
	// FIX 2026-08-11 (Vulkan dark floor, 1/4 — see Plan.md "D1").
	// NOTE: deliberately NOT gated on m_light->getActive(). The active
	// flags are refreshed by the scene stage's Optimize (render camera)
	// AFTER the shadow stages run, so gating here reads stale flags:
	// lights that shade this frame would sample never-rendered (zeroed)
	// shadow maps -> their contribution is multiplied to zero (the
	// "Vulkan dark floor" bug). The legacy GL pass path renders all
	// shadow maps unconditionally; pipeline mode now matches.


	if( !m_passToken )
		m_passToken = new lsStagePassToken( render, LSPT_SHADOWMAP );

	// --- Update phase (mirrors lsPass_ShadowMap::Update): the light is
	// the cull camera for the shadow view
	m_scene->InitDrawlist();
	m_scene->Optimize( m_light, m_passToken, m_scene, render );
	m_scene->OptimizeThread( m_light, m_passToken, m_scene, render );

	// --- Render phase (mirrors lsPass_ShadowMap::Render)
	m_fbo->Enable();

	render->Viewport( 0, 0, m_size, m_size );
	render->Enable( LS_DEPTH_TEST );
	render->DepthRangef( 0.0f, 1.0f );

	// push depth values away from the light to prevent shadow acne
	render->Enable( LS_POLYGON_OFFSET_FILL );
	render->PolygonOffset( 2.0f, 5.0f );

	m_fbo->ClearBuffers();

	m_scene->Draw( m_light, m_passToken, nullptr, render );

	// restore defaults for the next stage
	render->BindFramebuffer( LS_FRAMEBUFFER, lsTargetPool::SCREEN );
	render->Disable( LS_POLYGON_OFFSET_FILL );
	render->CullFace( LS_BACK );
}
