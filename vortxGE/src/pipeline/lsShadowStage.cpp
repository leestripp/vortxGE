#include "lsShadowStage.h"

#include "../datablocks/lsFBO_Depth.h"
#include "../scene/lsLight.h"
#include "../scene/lsScene.h"
#include "lsStagePassToken.h"

lsShadowStage::lsShadowStage( lsScene* scene, lsLight* light, int size,
							  lsRender* render )
	: m_scene( scene ), m_light( light ), m_size( size )
{
	if( !light || size <= 0 ) return;

	m_fbo = new lsFBO_Depth( (double)size, (double)size, render );

	// self-wire: hand the depth texture to the light for PBR sampling
	// (mirrors DemoScene::setupShadows)
	if( m_fbo )
	{
		light->setTex_shadowmap( m_fbo->getDepthTexture() );
		light->setHas_shadowmap( true );
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
	if( !m_scene || !m_light || !m_fbo ) return;
	if( !m_light->getActive() ) return;

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
