#include "lsShadowStage.h"
#include "../render/lsRenderTypes.h"

#include "../datablocks/lsFBO_Depth.h"
#include "../scene/lsLight.h"
#include "../scene/lsLight_Spot.h"
#include "../scene/lsObject.h"
#include "../scene/lsScene.h"

#include <vector>

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

lsShadowStage::lsShadowStage( lsScene* scene, lsLight* light,
							  lsShadowQuality quality, lsRender* render )
	: m_scene( scene ), m_light( light )
{
	m_shadowMap = new lsShadowMap( light, quality, render );
#if defined(VORTX_DEBUG_TOOLS)
	if( shadowTrace() )
		std::cout << "SHADOWSTAGE ctor light=" << ( light ? light->getName() : "(null)" )
				  << " type=" << ( light ? (int)light->getLight_type() : -1 )
				  << " size=" << m_shadowMap->size() << std::endl;
#endif
}

lsShadowStage::~lsShadowStage()
{
	delete m_shadowMap;
}

void lsShadowStage::execute( lsRender* render, lsTargetPool& targets )
{
	(void)targets;
	lsFBO_Depth* fbo = m_shadowMap ? m_shadowMap->fbo() : nullptr;
#if defined(VORTX_DEBUG_TOOLS)
	static uint32_t traceLeft = 12;
	if( shadowTrace() && traceLeft &&
		( !m_scene || !m_light || !fbo || !m_light->getActive() ) )
	{
		traceLeft--;
		std::cout << "SHADOWSTAGE skip light="
				  << ( m_light ? m_light->getName() : "(null)" )
				  << " scene=" << ( m_scene != nullptr )
				  << " fbo=" << ( fbo != nullptr )
				  << " active=" << ( m_light ? m_light->getActive() : false )
				  << std::endl;
	}
#endif
	if( !m_scene || !m_light || !fbo ) return;
	if( !m_light->cast_shadows ) return; // L3.3: per-light caster flag
	// FIX 2026-08-11 (Vulkan dark floor, 1/4 — see Plan.md "D1").
	// NOTE: deliberately NOT gated on m_light->getActive(). The active
	// flags are refreshed by the scene stage's Optimize (render camera)
	// AFTER the shadow stages run, so gating here reads stale flags:
	// lights that shade this frame would sample never-rendered (zeroed)
	// shadow maps -> their contribution is multiplied to zero (the
	// "Vulkan dark floor" bug). The legacy GL pass path renders all
	// shadow maps unconditionally; pipeline mode now matches.



	const int size = m_shadowMap->size();

	// --- Update phase (mirrors lsPass_ShadowMap::Update): the light is
	// the cull camera for the shadow view
	m_scene->InitDrawlist();
	m_scene->Optimize( m_light, m_passType, m_scene, render );
	m_scene->OptimizeThread( m_light, m_passType, m_scene, render );

	// L3.1 (spot): fit near/far to the culled drawlist content, then
	// rebuild the shadow matrix with the tight frustum. Culling above
	// ran with the previous (conservative) range — a superset — so
	// nothing the tight frustum needs was dropped. Per-object radius is
	// the scale magnitude capped at 2 m: huge flat meshes (the floor)
	// must not blow up the range.
	if( m_light->getLight_type() == LSLT_SPOT )
	{
		std::vector<lsVector3> positions;
		std::vector<float> radii;
		for( lsObject* o = m_scene->getDraw_list(); o;
			 o = o->getNext_drawobject() )
		{
			positions.push_back( o->ModelMatrix * lsVector3( 0.0f, 0.0f, 0.0f ) );
			const float s = o->scale.length();
			radii.push_back( s > 2.0f ? 2.0f : s );
		}
		float nearZ, farZ;
		if( lsFitSpotDepthRange( m_light->ViewMatrix, positions, radii,
								 nearZ, farZ ) )
		{
			m_light->Set_Perspective( m_light->spot_outer_cutoff * 2.0, 1.0,
									  nearZ, farZ );
			m_light->getShadowMatrix( render );
#if defined(VORTX_DEBUG_TOOLS)
			if( shadowTrace() )
				std::cout << "SHADOWSTAGE L3.1 fit light=" << m_light->getName()
						  << " near=" << nearZ << " far=" << farZ
						  << " casters=" << positions.size() << std::endl;
#endif
		}
	}

	// --- Render phase (mirrors lsPass_ShadowMap::Render)
	fbo->Enable();

	render->Viewport( 0, 0, size, size );
	render->Enable( LS_DEPTH_TEST );
	render->DepthRangef( 0.0f, 1.0f );

	// push depth values away from the light to prevent shadow acne
	render->Enable( LS_POLYGON_OFFSET_FILL );
	render->PolygonOffset( 2.0f, 5.0f );

	fbo->ClearBuffers();

	m_scene->Draw( m_light, m_passType, nullptr, render );

	// restore defaults for the next stage
	render->BindFramebuffer( LS_FRAMEBUFFER, lsTargetPool::SCREEN );
	render->Disable( LS_POLYGON_OFFSET_FILL );
	render->CullFace( LS_BACK );
}
