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

// opengl
#include "../opengl/lsOpenGL.h"

#include "lsFBO.h"
#include "lsFBO_ColourDepth_8.h"


lsFBO_ColourDepth_8::lsFBO_ColourDepth_8( int width, int height, lsOpenGL *opengl ) : lsFBO(opengl)
{
  setFBO_type( LSFT_COLOURDEPTH_8 );
  setWidth( width );
  setHeight( height );

    // colour Texture setup
    glGenTextures( 1, &texture );
    glBindTexture( GL_TEXTURE_2D, texture );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA,  width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    // glGenerateMipmap( GL_TEXTURE_2D );
    glBindTexture(GL_TEXTURE_2D, 0);

    // Depth texture
    glGenTextures( 1, &depth_texture );
    glBindTexture( GL_TEXTURE_2D, depth_texture );
    // GL_LINEAR does not make sense for depth texture. However, next tutorial shows usage of GL_LINEAR and PCF
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // Remove artefact on the edges
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    // No need to force GL_DEPTH_COMPONENT24, drivers usually give you the max precision if available
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Make sure default frame buffer is stored
    opengl->StoreFBO();
    // new FBO
    glGenFramebuffers( 1, &fbo );
    glBindFramebuffer( GL_FRAMEBUFFER, fbo );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0 );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0 );

    // Check all went well
    GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
    if( status == GL_FRAMEBUFFER_COMPLETE )
    {
        cout << "FBO created ID: " << fbo << endl;

    } else
    {
        cout << "ERROR: FBO creation failed *********************" << endl;
    }

    // Make sure default frame buffer is restored
    opengl->RestoreFBO();
}

lsFBO_ColourDepth_8::~lsFBO_ColourDepth_8()
{
  glDeleteFramebuffers( 1, &fbo );
  glDeleteTextures( 1, &depth_texture );
  glDeleteTextures( 1, &texture );
}

/**
 Alway store the old frame buffer before calling this and then restore stored frame buffer once done.
 */
void lsFBO_ColourDepth_8::Enable()
{
  glBindFramebuffer( GL_FRAMEBUFFER, fbo );
}

void lsFBO_ColourDepth_8::ClearBuffers()
{
  // Clear FBO
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

