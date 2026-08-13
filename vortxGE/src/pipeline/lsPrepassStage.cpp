#include "lsPrepassStage.h"

#include "../scene/lsCamera.h"
#include "../scene/lsScene.h"
#include "lsStagePassToken.h"

lsTextureID lsPrepassStage::s_depthTex = 0;

lsPrepassStage::lsPrepassStage( lsScene* scene, lsCamera* camera,
								const std::string& targetName )
	: m_scene( scene ), m_camera( camera ), m_target( targetName )
{
}

lsPrepassStage::~lsPrepassStage()
{
	delete m_passToken;
}

void lsPrepassStage::execute( lsRender* render, lsTargetPool& targets )
{
	s_depthTex = 0;
	if( !m_scene || !m_camera ) return;

	if( !m_passToken )
		m_passToken = new lsStagePassToken( render, LSPT_PREPASS );

	const lsFBOID fbo = targets.find( m_target );
	if( !fbo || fbo == lsTargetPool::SCREEN ) return; // target not acquired

	render->BindFramebuffer( LS_FRAMEBUFFER, fbo );
	render->Viewport( 0, 0, (lsSizei)targets.width(), (lsSizei)targets.height() );

	// complete state setup — nothing inherited from earlier stages
	render->ClearColor( 0.5, 0.5, 1.0, 0.0 ); // +Z normal = neutral
	render->ClearDepth( 1.0 );
	render->Enable( LS_DEPTH_TEST );
	render->DepthMask( LS_TRUE );
	render->Clear( LS_COLOR_BUFFER_BIT | LS_DEPTH_BUFFER_BIT );

	// same traversal as the main scene stage (visibility must match)
	m_scene->InitDrawlist();
	m_scene->Optimize( m_camera, m_passToken, m_scene, render );
	m_scene->OptimizeThread( m_camera, m_passToken, m_scene, render );
	m_scene->Draw( m_camera, m_passToken, m_scene, render );

	render->BindFramebuffer( LS_FRAMEBUFFER, lsTargetPool::SCREEN );
	s_depthTex = targets.depthTexture( m_target );
}
