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
	// L4.1: immutable storage (TexStorage2D) so the backend can alias a
	// raw-depth view for the PCSS blocker search (GL texture views
	// require it; VK treats it like a data-less TexImage2D).
	render->TexStorage2D( LS_TEXTURE_2D, 1, LS_DEPTH_COMPONENT, width, height );
	// L1.3: LINEAR + compare mode = hardware depth comparison with
	// bilinear PCF (4 taps per fetch) on both backends. Shaders sample
	// these maps as sampler2DShadow.
	render->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_MIN_FILTER, LS_LINEAR );
	render->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_MAG_FILTER, LS_LINEAR );
	render->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_COMPARE_MODE, LS_COMPARE_REF_TO_TEXTURE );
	render->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_COMPARE_FUNC, LS_LEQUAL );
	render->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_WRAP_S, LS_CLAMP_TO_BORDER );
	render->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_WRAP_T, LS_CLAMP_TO_BORDER );
	lsFloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	render->TexParameterfv( LS_TEXTURE_2D, LS_TEXTURE_BORDER_COLOR, borderColor );

	render->BindTexture( LS_TEXTURE_2D, 0 );

	// L4.1: raw (non-comparison) alias for the PCSS blocker search
	depth_texture_raw = render->CreateDepthRawView( depth_texture );

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
	if( depth_texture_raw ) render->DeleteTextures( 1, &depth_texture_raw ); // alias first
	render->DeleteTextures( 1, &depth_texture );

	depth_texture_raw = 0;
	depth_texture = 0;
	m_fbo = 0;

	setWidth( width );
	setHeight( height );

	CreateGL();
}

lsFBO_Depth::~lsFBO_Depth()
{
	if( getRender() )
	{
		if( depth_texture_raw ) getRender()->DeleteTextures( 1, &depth_texture_raw );
		getRender()->DeleteTextures( 1, &depth_texture );
	}
}

void lsFBO_Depth::Enable()
{
	getRender()->BindFramebuffer( LS_FRAMEBUFFER, m_fbo );
}

void lsFBO_Depth::ClearBuffers()
{
	getRender()->Clear( LS_DEPTH_BUFFER_BIT );
}
