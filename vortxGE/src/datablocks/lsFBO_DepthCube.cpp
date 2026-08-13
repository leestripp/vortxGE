#include <iostream>
#include "lsFBO_DepthCube.h"


lsFBO_DepthCube::lsFBO_DepthCube( int size, lsRender* render ) : lsFBO( render )
{
	setFBO_type( LSFT_DEPTH );
	setWidth( size );
	setHeight( size );

	CreateGL();
}

lsFBO_DepthCube::~lsFBO_DepthCube()
{
	if( getRender() )
	{
		getRender()->DeleteFramebuffers( 6, m_faceFbos );
		getRender()->DeleteTextures( 1, &depth_texture );
	}
}

void lsFBO_DepthCube::CreateGL()
{
	lsRender* render = getRender();
	const int size = (int)getWidth();

	// Cube depth texture (comparison sampler — shading uses
	// samplerCubeShadow, same L1.3 convention as the 2D maps)
	render->GenTextures( 1, &depth_texture );
	render->BindTexture( LS_TEXTURE_CUBE_MAP, depth_texture );
	for( int face = 0; face < 6; ++face )
		render->TexImage2D( LS_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0,
							LS_DEPTH_COMPONENT, size, size, 0,
							LS_DEPTH_COMPONENT, LS_FLOAT, NULL );
	render->TexParameteri( LS_TEXTURE_CUBE_MAP, LS_TEXTURE_MIN_FILTER, LS_LINEAR );
	render->TexParameteri( LS_TEXTURE_CUBE_MAP, LS_TEXTURE_MAG_FILTER, LS_LINEAR );
	render->TexParameteri( LS_TEXTURE_CUBE_MAP, LS_TEXTURE_COMPARE_MODE, LS_COMPARE_REF_TO_TEXTURE );
	render->TexParameteri( LS_TEXTURE_CUBE_MAP, LS_TEXTURE_COMPARE_FUNC, LS_LEQUAL );
	render->TexParameteri( LS_TEXTURE_CUBE_MAP, LS_TEXTURE_WRAP_S, LS_CLAMP_TO_EDGE );
	render->TexParameteri( LS_TEXTURE_CUBE_MAP, LS_TEXTURE_WRAP_T, LS_CLAMP_TO_EDGE );
	render->TexParameteri( LS_TEXTURE_CUBE_MAP, LS_TEXTURE_WRAP_R, LS_CLAMP_TO_EDGE );
	render->BindTexture( LS_TEXTURE_CUBE_MAP, 0 );

	// One framebuffer per face
	render->StoreDefaultFramebuffer();
	render->GenFramebuffers( 6, m_faceFbos );
	for( int face = 0; face < 6; ++face )
	{
		render->BindFramebuffer( LS_FRAMEBUFFER, m_faceFbos[face] );
		render->FramebufferTexture2D( LS_FRAMEBUFFER, LS_DEPTH_ATTACHMENT,
									  LS_TEXTURE_CUBE_MAP_POSITIVE_X + face,
									  depth_texture, 0 );
		render->DrawBuffer( LS_NONE );
		render->ReadBuffer( LS_NONE );
		if( render->CheckFramebufferStatus( LS_FRAMEBUFFER ) != LS_FRAMEBUFFER_COMPLETE )
			std::cout << "ERROR: DepthCube FBO face " << face
					  << " incomplete ********" << std::endl;
	}
	render->RestoreDefaultFramebuffer();
	std::cout << "DepthCube FBO created size=" << size << std::endl;
}

void lsFBO_DepthCube::EnableFace( int face )
{
	if( face < 0 || face > 5 ) return;
	getRender()->BindFramebuffer( LS_FRAMEBUFFER, m_faceFbos[face] );
}

void lsFBO_DepthCube::Resize( double width, double /*height*/ )
{
	if( (int)width == (int)getWidth() ) return;
	lsRender* render = getRender();
	if( !render ) return;

	render->DeleteFramebuffers( 6, m_faceFbos );
	render->DeleteTextures( 1, &depth_texture );
	depth_texture = 0;
	for( int i = 0; i < 6; ++i ) m_faceFbos[i] = 0;

	setWidth( width );
	setHeight( width );
	CreateGL();
}

void lsFBO_DepthCube::ClearBuffers()
{
	getRender()->Clear( LS_DEPTH_BUFFER_BIT );
}
