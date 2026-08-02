#include <iostream>
// debug
#include "../vortx_debug.h"

#include "lsFBO.h"

#include "lsFBO_ColourDepth_8.h"


lsFBO_ColourDepth_8::lsFBO_ColourDepth_8( double width, double height, lsRender *render ) : lsFBO(render)
{
	setFBO_type( LSFT_COLOURDEPTH_8 );
	setWidth( width );
	setHeight( height );

	// Allocate the colour + depth textures and attach them to a fresh FBO.
	CreateGL();
}

void lsFBO_ColourDepth_8::CreateGL()
{
	lsRender* render = getRender();
	double width  = getWidth();
	double height = getHeight();

	// colour Texture setup
	render->GenTextures( 1, &texture );
	render->BindTexture( LS_TEXTURE_2D, texture );
	render->TexImage2D( LS_TEXTURE_2D, 0, LS_RGBA,  width, height, 0, LS_RGBA, LS_UNSIGNED_BYTE, NULL );
	render->TexParameterf( LS_TEXTURE_2D, LS_TEXTURE_WRAP_S, LS_CLAMP_TO_EDGE );
	render->TexParameterf( LS_TEXTURE_2D, LS_TEXTURE_WRAP_T, LS_CLAMP_TO_EDGE );
	render->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_MAG_FILTER, LS_LINEAR );
	render->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_MIN_FILTER, LS_LINEAR );
	// render->GenerateMipmap( LS_TEXTURE_2D );
	render->BindTexture( LS_TEXTURE_2D, 0 );

	// Depth texture
	render->GenTextures( 1, &depth_texture );
	render->BindTexture( LS_TEXTURE_2D, depth_texture );
	// LS_LINEAR does not make sense for depth texture. However, next tutorial shows usage of LS_LINEAR and PCF
	render->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_MIN_FILTER, LS_NEAREST );
	render->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_MAG_FILTER, LS_NEAREST );
	// Remove artefact on the edges
	render->TexParameterf( LS_TEXTURE_2D, LS_TEXTURE_WRAP_S, LS_CLAMP_TO_EDGE );
	render->TexParameterf( LS_TEXTURE_2D, LS_TEXTURE_WRAP_T, LS_CLAMP_TO_EDGE );
	// No need to force LS_DEPTH_COMPONENT24, drivers usually give you the max precision if available
	render->TexImage2D( LS_TEXTURE_2D, 0, LS_DEPTH_COMPONENT, width, height, 0, LS_DEPTH_COMPONENT, LS_UNSIGNED_INT, NULL );
	render->BindTexture( LS_TEXTURE_2D, 0 );

	// Make sure default frame buffer is stored
	render->StoreDefaultFramebuffer();

	// Create FBO
	render->GenFramebuffers( 1, &m_fbo );
	render->BindFramebuffer( LS_FRAMEBUFFER, m_fbo );
	render->FramebufferTexture2D( LS_FRAMEBUFFER, LS_COLOR_ATTACHMENT0, LS_TEXTURE_2D, texture, 0 );
	render->FramebufferTexture2D( LS_FRAMEBUFFER, LS_DEPTH_ATTACHMENT, LS_TEXTURE_2D, depth_texture, 0 );

	// Check all went well
	if( render->CheckFramebufferStatus( LS_FRAMEBUFFER ) == LS_FRAMEBUFFER_COMPLETE )
	{
	  cout << "FBO created ID: " << m_fbo << endl;

	} else
	{
	  cout << "ERROR: FBO creation failed *********************" << endl;
	}

	// Make sure default frame buffer is restored
	render->RestoreDefaultFramebuffer();
}

void lsFBO_ColourDepth_8::Resize( double width, double height )
{
	if( width == getWidth() && height == getHeight() )
	{
#ifdef VORTX_DEBUG_RENDER
		cout << "lsFBO_ColourDepth_8::Resize - same size, skipping" << endl;
#endif
		return;
	}

#ifdef VORTX_DEBUG_RENDER
	cout << "lsFBO_ColourDepth_8::Resize " << (int)width << "x" << (int)height << endl;
#endif

	lsRender* render = getRender();
	if( !render ) return;

	// Tear down the old FBO + textures, then recreate at the new size.
	render->DeleteFramebuffers( 1, &m_fbo );
	render->DeleteTextures( 1, &depth_texture );
	render->DeleteTextures( 1, &texture );

	// Reset handle state before recreating.
	texture = 0;
	depth_texture = 0;
	m_fbo = 0;

	setWidth( width );
	setHeight( height );

	CreateGL();
}

lsFBO_ColourDepth_8::~lsFBO_ColourDepth_8()
{
  // FBO is deleted by subclass

  // delete textures
  if( getRender() )
  {
    getRender()->DeleteTextures( 1, &depth_texture );
    getRender()->DeleteTextures( 1, &texture );
  }
}

/**
 Alway store the old frame buffer before calling this and then restore stored frame buffer once done.
 */
void lsFBO_ColourDepth_8::Enable()
{
#ifdef VORTX_DEBUG_RENDER
	cout << "lsFBO_ColourDepth_8::Enable :" << m_fbo << endl;
#endif

  getRender()->BindFramebuffer( LS_FRAMEBUFFER, m_fbo );
}

void lsFBO_ColourDepth_8::ClearBuffers()
{
#ifdef VORTX_DEBUG_RENDER
	cout << "lsFBO_ColourDepth_8::ClearBuffers - Start" << endl;
#endif

  // Clear FBO
  getRender()->Clear( LS_COLOR_BUFFER_BIT | LS_DEPTH_BUFFER_BIT );
}

