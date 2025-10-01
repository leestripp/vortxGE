#include "../vortx_debug.h"

// datablocks
#include "../datablocks/lsString.h"

#include "lsOpenGL.h"

lsOpenGL::lsOpenGL() : lsRender()
{
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
    while( (err = glGetError()) != GL_NO_ERROR )
    {
        // Process/log the error.
        switch( err )
        {
        case GL_INVALID_ENUM:
            cout << "OpenGL error: " << name << " - GL_INVALID_ENUM" << endl;
            break;

        case GL_INVALID_FRAMEBUFFER_OPERATION:
            cout << "OpenGL error: " << name << " - GL_INVALID_FRAMEBUFFER_OPERATION" << endl;
            break;

        case GL_INVALID_INDEX:
            cout << "OpenGL error: " << name << " - GL_INVALID_INDEX" << endl;
            break;

        case GL_INVALID_OPERATION:
            cout << "OpenGL error: " << name << " - GL_INVALID_OPERATION" << endl;
            break;

        case GL_INVALID_VALUE:
            cout << "OpenGL error: " << name << " - GL_INVALID_VALUE" << endl;
            break;

        default:
            // Unknown error
            cout << "OpenGL error: " << name << " - Unknown error." << endl;
            break;
        }
    }
}


void lsOpenGL::glInfo()
{
#ifdef VORTX_DEBUG_INFO
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
#endif //VORTX_DEBUG_INFO
}

void lsOpenGL::StoreDefaultFramebuffer()
{
    // Find out default FBO
    glGetIntegerv( GL_FRAMEBUFFER_BINDING, &m_defaultFBO );

#ifdef VORTX_DEBUG_RENDER
    cout << "lsOpenGL::StoreFBO : " << default_FBO << endl;
#endif
}

void lsOpenGL::RestoreDefaultFramebuffer()
{
#ifdef VORTX_DEBUG_RENDER
    cout << "lsOpenGL::RestoreFBO : " << default_FBO << endl;
#endif
    glBindFramebuffer( GL_FRAMEBUFFER, m_defaultFBO );
}

