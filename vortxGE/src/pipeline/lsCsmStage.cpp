#include "lsCsmStage.h"

#include "../datablocks/lsFBO_Depth.h"
#include "../scene/lsCamera.h"
#include "../scene/lsLight.h"
#include "../scene/lsScene.h"
#include "lsStagePassToken.h"

#if defined(VORTX_DEBUG_TOOLS)
#include <cstdlib>
#include <iostream>
static bool csmTrace()
{
	static const bool v = std::getenv( "VORTX_CSM_TRACE" ) != nullptr;
	return v;
}
#endif

lsCsmStage::lsCsmStage( lsScene* scene, lsLight* light, lsCamera* camera,
						int mapSize, lsRender* render )
	: m_scene( scene ), m_light( light ), m_camera( camera )
{
	m_csm = new lsCascadedShadowMap( light, mapSize, render );
#if defined(VORTX_DEBUG_TOOLS)
	if( csmTrace() )
		std::cout << "CSMSTAGE ctor light=" << ( light ? light->getName() : "(null)" )
				  << " mapSize=" << mapSize
				  << " atlas=" << ( m_csm->atlas() != nullptr ) << std::endl;
#endif
}

lsCsmStage::~lsCsmStage()
{
	delete m_passToken;
	delete m_csm;
}

void lsCsmStage::execute( lsRender* render, lsTargetPool& targets )
{
	(void)targets;
	lsFBO_Depth* atlas = m_csm ? m_csm->atlas() : nullptr;
	if( !m_scene || !m_light || !m_camera || !atlas ) return;

	// sun direction: the light-camera's view -Z axis (back vector)
	const lsMatrix4x4& lvm = m_light->ViewMatrix;
	const lsVector3 lightDir( lvm.matrix[2], lvm.matrix[6], lvm.matrix[10] );

	m_csm->update( m_camera->ViewMatrix,
				   m_camera->getCam_fov(), m_camera->getCam_ratio(),
				   m_camera->getCam_near(), m_camera->getCam_far(),
				   -lightDir,	// light TRAVELS toward where it looks
				   render->getType() == RenderType::Vulkan );

	const auto& cascades = m_csm->cascades();
	if( cascades.empty() ) return;

	if( !m_passToken )
		m_passToken = new lsStagePassToken( render, LSPT_SHADOWMAP );

	atlas->Enable();
	render->Enable( LS_DEPTH_TEST );
	render->DepthRangef( 0.0f, 1.0f );
	render->Enable( LS_POLYGON_OFFSET_FILL );
	render->PolygonOffset( 2.0f, 5.0f );
	atlas->ClearBuffers();	// once for the whole atlas

	const int ms = m_csm->mapSize();

	for( size_t i = 0; i < cascades.size(); ++i )
	{
		const lsCsmCascade& cc = cascades[i];

		// throwaway cull/render camera carrying the cascade fit
		lsCamera cam( nullptr );	// throwaway, not in the scene graph
		cam.ViewMatrix = cc.view;
		const float r = cc.radius;
		cam.frustum.Set_Orthographic( -r, r, -r, r, -r, 3.0f * r );

		m_scene->InitDrawlist();
		m_scene->Optimize( &cam, m_passToken, m_scene, render );
		m_scene->OptimizeThread( &cam, m_passToken, m_scene, render );

		int qx, qy;
		m_csm->quadrant( (int)i, qx, qy );
		render->Viewport( qx, qy, ms, ms );

		m_scene->Draw( &cam, m_passToken, nullptr, render );
	}

	// restore defaults for the next stage
	render->BindFramebuffer( LS_FRAMEBUFFER, lsTargetPool::SCREEN );
	render->Disable( LS_POLYGON_OFFSET_FILL );
	render->CullFace( LS_BACK );
}
