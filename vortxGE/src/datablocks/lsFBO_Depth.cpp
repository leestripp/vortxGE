#include <iostream>
#include "../vortx_debug.h"
#include "lsFBO_Depth.h"


lsFBO_Depth::lsFBO_Depth( double width, double height, lsRender *render ) : lsFBO(render)
{
	setFBO_type( LSFT_DEPTH );
	setWidth( width );
	setHeight( height );

	CreateGL();
}

void lsFBO_Depth::CreateGL()
{
	lsRender* render = getRender();
	double width  = getWidth();
	double height = getHeight();

	// Depth texture
	render->GenTextures( 1, &depth_texture );
	render->BindTexture( LS_TEXTURE_2D, depth_texture );
	render->TexImage2D( LS_TEXTURE_2D, 0, LS_DEPTH_COMPONENT, width, height, 0, LS_DEPTH_COMPONENT, LS_FLOAT, NULL );
	render->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_MIN_FILTER, LS_NEAREST );
	render->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_MAG_FILTER, LS_NEAREST );
	render->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_WRAP_S, LS_CLAMP_TO_BORDER );
	render->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_WRAP_T, LS_CLAMP_TO_BORDER );
	lsFloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	render->TexParameterfv( LS_TEXTURE_2D, LS_TEXTURE_BORDER_COLOR, borderColor );

	render->BindTexture( LS_TEXTURE_2D, 0 );

	render->StoreDefaultFramebuffer();

	render->GenFramebuffers( 1, &m_fbo );
	render->BindFramebuffer( LS_FRAMEBUFFER, m_fbo );
	render->FramebufferTexture2D( LS_FRAMEBUFFER, LS_DEPTH_ATTACHMENT, LS_TEXTURE_2D, depth_texture, 0 );
	render->DrawBuffer( LS_NONE );
	render->ReadBuffer( LS_NONE );

	if( render->CheckFramebufferStatus( LS_FRAMEBUFFER ) == LS_FRAMEBUFFER_COMPLETE )
	{
	  cout << "Depth FBO created ID: " << m_fbo << endl;
	} else
	{
	  cout << "ERROR: Depth FBO creation failed *********************" << endl;
	}

	render->RestoreDefaultFramebuffer();
}

void lsFBO_Depth::Resize( double width, double height )
{
	if( width == getWidth() && height == getHeight() )
	{
#ifdef VORTX_DEBUG_RENDER
		cout << "lsFBO_Depth::Resize - same size, skipping" << endl;
#endif
		return;
	}

#ifdef VORTX_DEBUG_RENDER
	cout << "lsFBO_Depth::Resize " << (int)width << "x" << (int)height << endl;
#endif

	lsRender* render = getRender();
	if( !render ) return;

	render->DeleteFramebuffers( 1, &m_fbo );
	render->DeleteTextures( 1, &depth_texture );

	depth_texture = 0;
	m_fbo = 0;

	setWidth( width );
	setHeight( height );

	CreateGL();
}

lsFBO_Depth::~lsFBO_Depth()
{
	if( getRender() ) getRender()->DeleteTextures( 1, &depth_texture );
}

void lsFBO_Depth::Enable()
{
	getRender()->BindFramebuffer( LS_FRAMEBUFFER, m_fbo );
}

void lsFBO_Depth::ClearBuffers()
{
	getRender()->Clear( LS_DEPTH_BUFFER_BIT );
}
