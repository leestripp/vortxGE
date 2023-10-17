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

// opengl
#include "../opengl/lsOpenGL.h"

// shader
#include "../shader/lsShader.h"

#include "lsBatch_Quad.h"

// Data ***********************************

GLfloat vertices[] = {
    -1, -1, 0,
    -1,  1, 0,
    1,  1, 0,
    1, -1, 0
};

GLfloat uvs[] = {
    0, 0,
    0, 1,
    1, 1,
    1, 0
};

GLubyte indices[] = {
    0,1,2,
    0,2,3
};


// code ***********************************

lsBatch_Quad::lsBatch_Quad( lsOpenGL *opengl ) : lsBatch(opengl)
{
    setBatch_type( LSBT_QUAD );
    setName( "Batch Quad" );

    // Textures
    tex_diffuse = 0;

    // Create the buffer objects
    glGenBuffers( 1, &vbo_verts );
    glGenBuffers( 1, &vbo_uvs );
    glGenBuffers( 1, &vbo_indices );

    // Vertex data
    glBindBuffer( GL_ARRAY_BUFFER, vbo_verts );
    glBufferData( GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW );

    // uv data
    glBindBuffer( GL_ARRAY_BUFFER, vbo_uvs );
    glBufferData( GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW );

    // Indexes
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbo_indices );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW );

}

lsBatch_Quad::~lsBatch_Quad()
{
#if defined(VORTX_DEBUG_BATCH)
    cout << "lsBatch_Quad::~lsBatch_Quad : Free GL buffers " << getName() << endl;
#endif
    // Delete buffer objects
    glDeleteBuffers( 1, &vbo_uvs );
    glDeleteBuffers( 1, &vbo_verts );
    glDeleteBuffers( 1, &vbo_indices );
}

void lsBatch_Quad::Draw( lsCamera* cam, lsObject *object, lsPass *pass )
{
#ifdef VORTX_DEBUG_RENDER
    cout << "lsBatch_Quad::Draw" << endl;
#endif

    GLuint vertex, uv;

    vertex = 0;
    uv = 0;

    if( getShader() )
    {
        getShader()->UseShader( cam, object, pass, this );
        vertex = glGetAttribLocation( getShader()->getProgram(), "vxVertex" );
        uv = glGetAttribLocation( getShader()->getProgram(), "vxTexCoord" );
    } else
    {
        cout << "lsBatch_Quad::Draw - Shader missing" << endl;
    }

    // Verts
    glBindBuffer( GL_ARRAY_BUFFER, vbo_verts );
    glEnableVertexAttribArray( vertex );
    glVertexAttribPointer( vertex, 3, GL_FLOAT, GL_FALSE, 0, 0 );

    // TexCoords
    glBindBuffer( GL_ARRAY_BUFFER, vbo_uvs );
    glEnableVertexAttribArray( uv );
    glVertexAttribPointer( uv, 2, GL_FLOAT, GL_FALSE, 0, 0 );

    // Indexes
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbo_indices );

    glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0 );

    glDisableVertexAttribArray( vertex );
    glDisableVertexAttribArray( uv );

    if( getShader() ) glUseProgram( 0 );

    // Pass it on
    if( getNext_batch() ) getNext_batch()->Draw( cam, object, pass );
}

void lsBatch_Quad::Update( double delta )
{
    // Not used

    // Pass it on
    if( getNext_batch() ) getNext_batch()->Update( delta );
}

void lsBatch_Quad::Copy_GLdata()
{
}

void lsBatch_Quad::Clear_GLdata()
{
}

