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
    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    glewInit();

#ifdef VORTX_DEBUG_OPENGLERRORS
    cout << "lsOpenGL::Init() - Check GL errors" << endl;
    CheckGLErrors();
#endif // VORTX_DEBUG_OPENGLERRORS

    // TODO: Make these options.
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

