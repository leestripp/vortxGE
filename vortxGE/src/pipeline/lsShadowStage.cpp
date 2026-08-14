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

	// SHADOW-STABILITY FIX (2026-08-14): world-anchored texel snapping
	// for DIRECT lights. The demo sun follows the camera (followCamera),
	// which recenters its ortho shadow map every frame; without snapping,
	// every camera move slides all shadow texels across the world and the
	// shadows shimmer/flicker. Snap the light so the FIXED WORLD ORIGIN
	// lands exactly on a shadow-texel boundary: texels stay glued to
	// world positions and camera motion no longer moves the shadows.
	// (x/y kill the sliding; z snapping keeps depth stable -> no acne
	// flicker.) Spots/points are light-centric and already stable.
	if( m_light->getLight_type() == LSLT_DIRECT && size > 0 )
	{
		const float wptX = ( m_light->getOrtho_right() - m_light->getOrtho_left() ) / (float)size;
		const float wptY = ( m_light->getOrtho_top() - m_light->getOrtho_bottom() ) / (float)size;
		const float wptZ = ( m_light->getCam_far() - m_light->getCam_near() ) / (float)size;
		if( wptX > 0.0f && wptY > 0.0f && wptZ > 0.0f )
		{
			const lsMatrix4x4& V = m_light->ViewMatrix;
			// light-space position of the world origin == ViewMatrix
			// translation column (V * (0,0,0,1))
			const float lx = V.matrix[12], ly = V.matrix[13], lz = V.matrix[14];
			const float dx = std::floor( lx / wptX ) * wptX - lx;
			const float dy = std::floor( ly / wptY ) * wptY - ly;
			const float dz = std::floor( lz / wptZ ) * wptZ - lz;
			if( dx != 0.0f || dy != 0.0f || dz != 0.0f )
			{
				// light-space delta -> world delta via R^T (rows of the
				// upper 3x3, column-major storage)
				m_light->ModelMatrix.matrix[12] += V.matrix[0]*dx + V.matrix[1]*dy + V.matrix[2]*dz;
				m_light->ModelMatrix.matrix[13] += V.matrix[4]*dx + V.matrix[5]*dy + V.matrix[6]*dz;
				m_light->ModelMatrix.matrix[14] += V.matrix[8]*dx + V.matrix[9]*dy + V.matrix[10]*dz;
				m_light->Update_Camera(); // rebuild ViewMatrix snapped
			}
#if defined(VORTX_DEBUG_TOOLS)
			if( shadowTrace() )
			{
				static int s_snapTrace = 0;
				if( s_snapTrace++ % 30 == 0 )
					std::cout << "SHADOWSTAGE snap light=" << m_light->getName()
					          << " pos=(" << m_light->ModelMatrix.matrix[12] << ", "
					          << m_light->ModelMatrix.matrix[13] << ", "
					          << m_light->ModelMatrix.matrix[14] << ")"
					          << " wpt=(" << wptX << ")" << std::endl;
			}
#endif
		}
	}

	// --- Update phase: the light is
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
			m_light->buildShadowmap_matrix( render );
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
