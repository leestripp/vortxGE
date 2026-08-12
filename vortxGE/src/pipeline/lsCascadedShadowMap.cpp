#include "lsCascadedShadowMap.h"

#include "../datablocks/lsFBO_Depth.h"
#include "../scene/lsLight.h"

lsCascadedShadowMap::lsCascadedShadowMap( lsLight* light, int mapSize,
										  lsRender* render )
	: m_light( light ), m_mapSize( mapSize )
{
	if( !light || mapSize <= 0 ) { m_mapSize = 0; return; }

	m_config.mapSize = mapSize;
	m_atlas = new lsFBO_Depth( 2.0 * mapSize, 2.0 * mapSize, render );

	// self-wire (sampling side lands with L2.4; wire now so debug
	// readbacks treat the atlas as the light's shadow map)
	if( m_atlas )
	{
		light->setTex_shadowmap( m_atlas->getDepthTexture() );
		light->setHas_shadowmap( true );
		light->setPixel_x( 2.0f * mapSize );
		light->setPixel_y( 2.0f * mapSize );
	}
}

lsCascadedShadowMap::~lsCascadedShadowMap()
{
	delete m_atlas;
}

void lsCascadedShadowMap::update( const lsMatrix4x4& camView,
								  float fovYDeg, float aspect,
								  float nearZ, float farZ,
								  const lsVector3& lightDir, bool yFlip )
{
	m_cascades = lsCsmCompute( m_config, camView, fovYDeg, aspect,
							   nearZ, farZ, lightDir, yFlip );
}
