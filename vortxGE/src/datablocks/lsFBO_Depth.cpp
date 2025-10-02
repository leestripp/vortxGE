#include <iostream>
#include "../vortx_debug.h"
#include "lsFBO_Depth.h"

lsFBO_Depth::lsFBO_Depth( double width, double height, lsRender *render ) : lsFBO(render)
{
	setFBO_type( LSFT_DEPTH );
	setWidth( width );
	setHeight( height );

	// Depth texture
	glGenTextures( 1, &depth_texture );
	glBindTexture( GL_TEXTURE_2D, depth_texture );
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindTexture(GL_TEXTURE_2D, 0);

	render->StoreDefaultFramebuffer();

	glGenFramebuffers( 1, &m_fbo );
	glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0 );
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	if( status == GL_FRAMEBUFFER_COMPLETE )
	{
	  cout << "Depth FBO created ID: " << m_fbo << endl;
	} else
	{
	  cout << "ERROR: Depth FBO creation failed *********************" << endl;
	}

	render->RestoreDefaultFramebuffer();
}

lsFBO_Depth::~lsFBO_Depth()
{
	glDeleteTextures( 1, &depth_texture );
}

void lsFBO_Depth::Enable()
{
	glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
}

void lsFBO_Depth::ClearBuffers()
{
	glClear( GL_DEPTH_BUFFER_BIT );
}
