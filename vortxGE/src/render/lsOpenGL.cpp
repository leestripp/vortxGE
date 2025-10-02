#include "../vortx_debug.h"

// datablocks
#include "../datablocks/lsString.h"

#include "lsOpenGL.h"

lsOpenGL::lsOpenGL() : lsRender()
{
	setType(RenderType::OpenGL);
	
	// Initialize OpenGL-specific members
	m_defaultFBO = 0;
}

lsOpenGL::~lsOpenGL()
{
}


void lsOpenGL::Init()
{
#ifdef VORTX_DEBUG_RENDER
	cout << "lsOpenGL::Init()" << endl;
#endif

	// Extensions
	// User needs to load extensions e.g. GLEW
	
	// collect info
	glInfo();
}

void lsOpenGL::CheckErrors( const string &name )
{
	GLenum err;
	while((err = glGetError()) != GL_NO_ERROR)
	{
		string errorMsg = "Unknown error";
		switch(err)
		{
			case GL_INVALID_ENUM:           errorMsg = "GL_INVALID_ENUM"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: errorMsg = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
			case GL_INVALID_INDEX:          errorMsg = "GL_INVALID_INDEX"; break;
			case GL_INVALID_OPERATION:      errorMsg = "GL_INVALID_OPERATION"; break;
			case GL_INVALID_VALUE:          errorMsg = "GL_INVALID_VALUE"; break;
		}
		cerr << "OpenGL ERROT : " << name << " : " << errorMsg << endl;
	}
}

void lsOpenGL::glInfo()
{
	cout << "***** ******************************************" << endl;
	cout << "***** OpenGL Info" << endl;
	cout << "***** ******************************************" << endl;

	// Version
	cout << "* Version  : ";
	cout << glGetString(GL_VERSION) << endl;

	// Vender
	cout << "* Vender   : ";
	cout << glGetString( GL_VENDOR ) << endl;

	// Renderer
	cout << "* Renderer : ";
	cout << glGetString( GL_RENDERER ) << endl;

	// GL_SHADING_LANGUAGE_VERSION​​
	cout << "* Shaders  : ";
	cout << glGetString( GL_SHADING_LANGUAGE_VERSION ) << endl;

	cout << "***** ******************************************" << endl;
}

void lsOpenGL::StoreDefaultFramebuffer()
{
	// Find out default FBO
	glGetIntegerv( GL_FRAMEBUFFER_BINDING, &m_defaultFBO );
	
#ifdef VORTX_DEBUG_RENDER
	cout << "lsOpenGL::StoreDefaultFramebuffer : " << m_defaultFBO << endl;
#endif
}

void lsOpenGL::RestoreDefaultFramebuffer()
{
#ifdef VORTX_DEBUG_RENDER
	cout << "lsOpenGL::RestoreDefaultFramebuffer : " << m_defaultFBO << endl;
#endif
	glBindFramebuffer( GL_FRAMEBUFFER, m_defaultFBO );
}

