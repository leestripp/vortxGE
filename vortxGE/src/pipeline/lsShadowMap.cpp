#include "lsShadowMap.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

#include "../datablocks/lsFBO_Depth.h"
#include "../scene/lsLight.h"

using namespace std;

static lsShadowQuality qualityFromEnv()
{
	const char* env = std::getenv( "VORTX_SHADOW_QUALITY" );
	if( !env ) return lsShadowQuality::High;
	if( !strcmp( env, "low" ) )  return lsShadowQuality::Low;
	if( !strcmp( env, "med" ) )  return lsShadowQuality::Medium;
	return lsShadowQuality::High;
}

static lsShadowQuality g_quality = qualityFromEnv();

void lsShadowMap::setQuality( lsShadowQuality q ) { g_quality = q; }
lsShadowQuality lsShadowMap::getQuality() { return g_quality; }

int lsShadowMap::resolutionFor( lsLight* light, lsShadowQuality q )
{
	if( !light ) return 0;
	switch( light->getLight_type() )
	{
	case LSLT_DIRECT:
		return q == lsShadowQuality::High ? 4096 :
			   q == lsShadowQuality::Medium ? 2048 : 1024;
	case LSLT_SPOT:
		return q == lsShadowQuality::High ? 2048 :
			   q == lsShadowQuality::Medium ? 1024 : 512;
	default:
		return 0;	// point lights: cube maps are L2 territory
	}
}

lsShadowMap::lsShadowMap( lsLight* light, lsShadowQuality quality,
						  lsRender* render )
	: m_light( light )
{
	m_size = resolutionFor( light, quality );
	if( !light || m_size <= 0 ) return;

	m_fbo = new lsFBO_Depth( (double)m_size, (double)m_size, render );
	if( !m_fbo ) return;

	// self-wire: hand the depth texture to the light for PBR sampling
	light->setTex_shadowmap( m_fbo->getDepthTexture() );
	light->setTex_shadowmap_raw( m_fbo->getDepthTextureRaw() ); // L4.1
	light->setHas_shadowmap( true );
	light->setPixel_x( m_size );	// L1.5: texel snapping needs the size
	light->setPixel_y( m_size );
}

lsShadowMap::~lsShadowMap()
{
	delete m_fbo;
}
