/*
 * Copyright (c) 2016 by Lee A. Stripp <leestripp@gmail.com>
 *
 * This file is part of the vortxGE library.
 *
 * vortxGE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * vortxGE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with vortxGE.  If not, see <http://www.gnu.org/licenses/>
 */

#include "../vortx_debug.h"

// datablocks
#include "../datablocks/lsString.h"

#include "lsOpenGL.h"

lsOpenGL::lsOpenGL()
{
    shadermanager = NULL;
    stats = NULL;
}

lsOpenGL::~lsOpenGL()
{
}

/**
 * @brief Sets up openGL extensions and displays basic info.
 *
 * You should have an active context before calling this function.
 * Automaticlly called by vortxGE::Init().
 *
 * @return void
 */
void lsOpenGL::Init()
{
#ifdef VORTX_DEBUG_RENDER
    cout << "lsOpenGL::Init()" << endl;
#endif

    // Extensions
    LoadExtensions();

#ifdef VORTX_DEBUG_OPENGLERRORS
    cout << "lsOpenGL::Init() - Check GL errors" << endl;
    CheckGLErrors();
#endif // VORTX_DEBUG_OPENGLERRORS

    // Draw direction and face cull
    glFrontFace( GL_CCW );
    glCullFace( GL_BACK );
    glEnable( GL_CULL_FACE );

    // collect info
    glInfo();
}

void lsOpenGL::CheckGLErrors()
{
    GLenum err;
    while( (err = glGetError()) != GL_NO_ERROR )
    {
        // Process/log the error.
        switch( err )
        {
        case GL_INVALID_ENUM:
            cout << "OpenGL error: GL_INVALID_ENUM" << endl;
            break;

        case GL_INVALID_FRAMEBUFFER_OPERATION:
            cout << "OpenGL error: GL_INVALID_FRAMEBUFFER_OPERATION" << endl;
            break;

        case GL_INVALID_INDEX:
            cout << "OpenGL error: GL_INVALID_INDEX" << endl;
            break;

        case GL_INVALID_OPERATION:
            cout << "OpenGL error: GL_INVALID_OPERATION" << endl;
            break;

        case GL_INVALID_VALUE:
            cout << "OpenGL error: GL_INVALID_VALUE" << endl;
            break;

        default:
            // Unknown error
            break;
        }
    }
}


/**
 * @brief Basic openGL information
 *
 * Called by lsOpenGL::Init(), so there is no deed to call it again.
 * You need a current context for this to work correctly.
 *
 * @return void
 */
void lsOpenGL::glInfo()
{
#ifdef VORTX_DEBUG_INFO
    cout << "***** ******************************************" << endl;
    cout << "***** OpenGL Info" << endl;
    cout << "***** ******************************************" << endl;

    // Version
    GLint major_version;
    GLint minor_version;
    glGetIntegerv( GL_MAJOR_VERSION, &major_version );
    glGetIntegerv( GL_MINOR_VERSION, &minor_version );
    cout << "* Context version : ";
    cout << major_version << "." << minor_version << endl;

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


/**
 * @brief Load basic extensions needed by the engine
 *
 * This is called by the lsOpenGL::Init() function, no need to call it yourself.
 *
 * @return void
 */
void lsOpenGL::LoadExtensions()
{
    // FBO
    m_hasFBO = false;
    if( (glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)glXGetProcAddress( (const GLubyte*)"glGenFramebuffers" )) )
    {
        m_hasFBO = true;
        glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)glXGetProcAddress( (const GLubyte*)"glBindFramebuffer" );
        glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)glXGetProcAddress( (const GLubyte*)"glFramebufferTexture2D" );
        glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)glXGetProcAddress( (const GLubyte*)"glDeleteFramebuffers" );
        glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)glXGetProcAddress( (const GLubyte*)"glCheckFramebufferStatus" );

    } else
    {
        cout << "Unable to load FBO extensions" << endl;
    }

    // VBOs
    m_hasVBO = false;
    if( (glGenBuffers = (PFNGLGENBUFFERSPROC)glXGetProcAddress( (const GLubyte*)"glGenBuffers" )) )
    {
        m_hasVBO = true;
        glBindBuffer = (PFNGLBINDBUFFERPROC)glXGetProcAddress( (const GLubyte*)"glBindBuffer" );
        glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)glXGetProcAddress( (const GLubyte*)"glDeleteBuffers" );
        glBufferData = (PFNGLBUFFERDATAPROC)glXGetProcAddress( (const GLubyte*)"glBufferData" );

    } else
    {
        cout << "Unable to load VBO extensions" << endl;
    }

    // Attrs
    if( (glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)glXGetProcAddress( (const GLubyte*)"glGetAttribLocation" )) )
    {
        glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)glXGetProcAddress( (const GLubyte*)"glEnableVertexAttribArray" );
        glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)glXGetProcAddress( (const GLubyte*)"glVertexAttribPointer" );
        glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)glXGetProcAddress( (const GLubyte*)"glDisableVertexAttribArray" );


    } else
    {
        cout << "Unable to load Attrib extensions" << endl;
    }

    // Shaders
    m_hasShaders = false;
    if( (glUseProgram = (PFNGLUSEPROGRAMPROC)glXGetProcAddress( (const GLubyte*)"glUseProgram" )) )
    {
        m_hasShaders = true;
        glDeleteProgram = (PFNGLDELETEPROGRAMPROC)glXGetProcAddress( (const GLubyte*)"glDeleteProgram" );
        glCreateShader = (PFNGLCREATESHADERPROC)glXGetProcAddress( (const GLubyte*)"glCreateShader" );
        glDetachShader = (PFNGLDETACHSHADERPROC)glXGetProcAddress( (const GLubyte*)"glDetachShader" );
        glDeleteShader = (PFNGLDELETESHADERPROC)glXGetProcAddress( (const GLubyte*)"glDeleteShader" );
        glCreateProgram = (PFNGLCREATEPROGRAMPROC)glXGetProcAddress( (const GLubyte*)"glCreateProgram" );
        glAttachShader = (PFNGLATTACHSHADERPROC)glXGetProcAddress( (const GLubyte*)"glAttachShader" );
        glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)glXGetProcAddress( (const GLubyte*)"glGetUniformLocation" );
        glShaderSource = (PFNGLSHADERSOURCEPROC)glXGetProcAddress( (const GLubyte*)"glShaderSource" );
        glCompileShader = (PFNGLCOMPILESHADERPROC)glXGetProcAddress( (const GLubyte*)"glCompileShader" );
        glLinkProgram = (PFNGLLINKPROGRAMPROC)glXGetProcAddress( (const GLubyte*)"glLinkProgram" );
        glGetShaderiv = (PFNGLGETSHADERIVPROC)glXGetProcAddress( (const GLubyte*)"glGetShaderiv" );
        glGetProgramiv = (PFNGLGETPROGRAMIVPROC)glXGetProcAddress( (const GLubyte*)"glGetProgramiv" );
        glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)glXGetProcAddress( (const GLubyte*)"glGetShaderInfoLog" );
        glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)glXGetProcAddress( (const GLubyte*)"glGetProgramInfoLog" );


    } else
    {
        cout << "Unable to load Shader extensions" << endl;
    }

    // Textures
    if( (glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)glXGetProcAddress( (const GLubyte*)"glGenerateMipmap" )) )
    {
        // glUniform1i = (PFNGLUNIFORM1IPROC)glXGetProcAddress( (const GLubyte*)"glUniform1i" );

    } else
    {
        cout << "Unable to load Texture extensions" << endl;
    }

    // Queries
    if( (glGenQueries = (PFNGLGENQUERIESPROC)glXGetProcAddress( (const GLubyte*)"glGenQueries" )) )
    {
        glBeginQuery = (PFNGLBEGINQUERYPROC)glXGetProcAddress( (const GLubyte*)"glBeginQuery" );
        glEndQuery = (PFNGLENDQUERYPROC)glXGetProcAddress( (const GLubyte*)"glEndQuery" );
        glGetQueryObjectiv = (PFNGLGETQUERYOBJECTIVPROC)glXGetProcAddress( (const GLubyte*)"glGetQueryObjectiv" );
        glDeleteQueries = (PFNGLDELETEQUERIESPROC)glXGetProcAddress( (const GLubyte*)"glDeleteQueries" );

    } else
    {
        cout << "Unable to load Queries extensions" << endl;
    }

    // Uniforms
    if( (glUniform1i = (PFNGLUNIFORM1IPROC)glXGetProcAddress( (const GLubyte*)"glUniform1i" )) )
    {
        glUniform4f = (PFNGLUNIFORM4FPROC)glXGetProcAddress( (const GLubyte*)"glUniform4f" );
        glUniform1f = (PFNGLUNIFORM1FPROC)glXGetProcAddress( (const GLubyte*)"glUniform1f" );
        glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)glXGetProcAddress( (const GLubyte*)"glUniformMatrix4fv" );
        glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)glXGetProcAddress( (const GLubyte*)"glUniformMatrix3fv" );
        glUniform3f = (PFNGLUNIFORM3FPROC)glXGetProcAddress( (const GLubyte*)"glUniform3f" );
        glUniform2f = (PFNGLUNIFORM2FPROC)glXGetProcAddress( (const GLubyte*)"glUniform2f" );

    } else
    {
        cout << "Unable to load Uniform extensions" << endl;
    }

}


/**
 * @brief Convenience function for storing the current Frame Buffer object
 *
 * @return void
 */
void lsOpenGL::StoreFBO()
{
    // Find out default FBO
    glGetIntegerv( GL_FRAMEBUFFER_BINDING, &default_FBO );

#ifdef VORTX_DEBUG_RENDER
    cout << "lsOpenGL::StoreFBO : " << default_FBO << endl;
#endif
}

/**
 * @brief Convenience function to restore a stored Frame Buffer object
 *
 * @return void
 */

void lsOpenGL::RestoreFBO()
{
#ifdef VORTX_DEBUG_RENDER
    cout << "lsOpenGL::RestoreFBO : " << default_FBO << endl;
#endif
    glBindFramebuffer( GL_FRAMEBUFFER, default_FBO );
}

