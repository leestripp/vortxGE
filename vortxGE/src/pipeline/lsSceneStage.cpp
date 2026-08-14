#include "lsSceneStage.h"
#include "../render/lsRenderTypes.h"

#include "../scene/lsCamera.h"
#include "../scene/lsScene.h"

lsSceneStage::lsSceneStage( lsScene* scene, lsCamera* camera,
							const std::string& targetName )
	: m_scene( scene ), m_camera( camera ), m_target( targetName )
{
}

lsSceneStage::~lsSceneStage()
{
}

void lsSceneStage::execute( lsRender* render, lsTargetPool& targets )
{
	// E3.2/R5: fall back to the scene's active camera when no explicit
	// camera is set — stages need not be wired to control classes.
	if( !m_camera && m_scene ) m_camera = m_scene->getActiveCamera();
	if( !m_scene || !m_camera ) return;

	// lazily create the pass token (needs the render interface)

	const bool toScreen =
		( m_target.empty() || m_target == "screen" );
	const lsFBOID fbo = toScreen ? lsTargetPool::SCREEN
								 : targets.find( m_target );

	render->BindFramebuffer( LS_FRAMEBUFFER, fbo );

	// viewport: normalized rect x framebuffer (single resize owner)
	const float tw = (float)targets.width();
	const float th = (float)targets.height();
	render->Viewport( (lsInt)( m_rect[0] * tw ), (lsInt)( m_rect[1] * th ),
					  (lsSizei)( m_rect[2] * tw ), (lsSizei)( m_rect[3] * th ) );

	// keep the camera aspect/frustum matched to the stage viewport
	// (legacy passes did this in Resize; the pipeline owns it now —
	// Plan.md Part 3 rule 5). A stale ratio stretches the projection,
	// which skews perceived movement angles.
	const float vw = m_rect[2] * tw, vh = m_rect[3] * th;
	if( vw > 0.0f && vh > 0.0f )
		m_camera->Set_ViewRatio( vw / vh );

	// complete state setup — nothing inherited from earlier stages
	render->ClearColor( m_clear[0], m_clear[1], m_clear[2], m_clear[3] );
	render->ClearDepth( 1.0 );
	render->Enable( LS_DEPTH_TEST );
	render->DepthMask( LS_TRUE );
	render->Clear( LS_COLOR_BUFFER_BIT | LS_DEPTH_BUFFER_BIT );
	render->BlendFunc( LS_SRC_ALPHA, LS_ONE_MINUS_SRC_ALPHA );

	// per-frame scene traversal (mirrors lsPass_FBO::Update+Render:
	// build draw list, optimize lights, cull drawables, then draw)
	m_scene->InitDrawlist();
	m_scene->Optimize( m_camera, m_passType, m_scene, render );
	m_scene->OptimizeThread( m_camera, m_passType, m_scene, render );
	m_scene->Draw( m_camera, m_passType, m_scene, render );

	if( !toScreen )
		render->BindFramebuffer( LS_FRAMEBUFFER, lsTargetPool::SCREEN );
}
