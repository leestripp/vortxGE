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

#include <iostream>
#include <cstring>

#include "../vortx_debug.h"

// opengl
#include "../opengl/lsOpenGL.h"

// shader
#include "../shader/lsShader.h"

#include "lsBatch_Lines.h"


lsBatch_Lines::lsBatch_Lines( lsOpenGL *opengl ) : lsBatch(opengl)
{
    setBatch_type( LSBT_LINE );
    setName("Batch Lines");

    nNumVerts = 0;

    // Temp Data
    pVerts = NULL;
    pColours = NULL;
}

lsBatch_Lines::~lsBatch_Lines()
{
#if defined(VORTX_DEBUG_BATCH)
    cout << "lsBatch_Lines::~lsBatch_Lines : Free GL buffers " << getName() << endl;
#endif
    // Cleanup
    Cleanup();
    Clear_GLdata();
}


void lsBatch_Lines::Begin( GLuint nMaxVerts )
{
    // cleanup
    Cleanup();
    nNumVerts = 0;

    // Alloc
    pVerts = new Vector3f[ nMaxVerts ];
    pColours = new Vector4f[ nMaxVerts ];

}

void lsBatch_Lines::AddLine( Vector3f verts[2], Vector4f vColours[2] )
{
    memcpy( pVerts[nNumVerts], verts[0], sizeof(Vector3f) );
    memcpy( pColours[nNumVerts], vColours[0], sizeof(Vector4f) );
    nNumVerts++;
    memcpy( pVerts[nNumVerts], verts[1], sizeof(Vector3f) );
    memcpy( pColours[nNumVerts], vColours[1], sizeof(Vector4f) );
    nNumVerts++;

}

void lsBatch_Lines::End()
{
    Copy_GLdata();
}

void lsBatch_Lines::Cleanup()
{
    // Cleanup
    if( pVerts ) delete [] pVerts;
    if( pColours ) delete [] pColours;

    pVerts = NULL;
    pColours = NULL;
}

void lsBatch_Lines::Draw( lsCamera *cam, lsObject *object, lsPass *pass )
{
#ifdef VORTX_DEBUG_RENDER
    cout << "lsBatch_Lines::Draw" << endl;
#endif

    // cout << "lsBatch_Lines::Draw()" << endl;
    GLuint vertex, colour;

    vertex = 0;
    colour = 0;

    if( getShader() )
    {
        getShader()->UseShader( cam, object, pass, this );
        vertex = getOgl()->glGetAttribLocation( getShader()->getProgram(), "vxVertex" );
        colour = getOgl()->glGetAttribLocation( getShader()->getProgram(), "vxColour" );
    }else
    {
        cout << "lsBatch_Lines::Draw - Shader missing" << endl;
    }

    // Verts
    getOgl()->glBindBuffer( GL_ARRAY_BUFFER, vbo_verts );
    getOgl()->glEnableVertexAttribArray( vertex );
    getOgl()->glVertexAttribPointer( vertex, 3, GL_FLOAT, GL_FALSE, 0, 0 );

    // Colours
    getOgl()->glBindBuffer( GL_ARRAY_BUFFER, vbo_colours );
    getOgl()->glEnableVertexAttribArray( colour );
    getOgl()->glVertexAttribPointer( colour, 4, GL_FLOAT, GL_FALSE, 0, 0 );

    glDrawArrays( GL_LINES, 0, nNumVerts );

    getOgl()->glDisableVertexAttribArray( vertex );
    getOgl()->glDisableVertexAttribArray( colour );

    if( getShader() ) getOgl()->glUseProgram(0);

    // Pass it on
    if( getNext_batch() ) getNext_batch()->Draw( cam, object, pass );
}

void lsBatch_Lines::Update( double delta )
{
    // Not used

    // Pass it on
    if( getNext_batch() ) getNext_batch()->Update( delta );
}

void lsBatch_Lines::buildEmpty(GLfloat scale)
{
    Vector3f vert[2];
    Vector4f colour[2];

    Begin( 6 );

    // X direction
    // Line verts
    vert[0][0] = 1.0*scale;
    vert[0][1] = 0.0;
    vert[0][2] = 0.0;

    vert[1][0] = 0.0;
    vert[1][1] = 0.0;
    vert[1][2] = 0.0;

    // Colours
    colour[0][0] = 1.0;
    colour[0][1] = 0.0;
    colour[0][2] = 0.0;
    colour[0][3] = 1.0;

    colour[1][0] = 1.0;
    colour[1][1] = 0.0;
    colour[1][2] = 0.0;
    colour[1][3] = 1.0;

    AddLine( vert, colour );

    // Y direction
    // Line verts
    vert[0][0] = 0.0;
    vert[0][1] = 1.0*scale;
    vert[0][2] = 0.0;

    vert[1][0] = 0.0;
    vert[1][1] = 0.0;
    vert[1][2] = 0.0;

    // Colours
    colour[0][0] = 0.0;
    colour[0][1] = 1.0;
    colour[0][2] = 0.0;
    colour[0][3] = 1.0;

    colour[1][0] = 0.0;
    colour[1][1] = 1.0;
    colour[1][2] = 0.0;
    colour[1][3] = 1.0;

    AddLine( vert, colour );

    // Z direction
    // Line verts
    vert[0][0] = 0.0;
    vert[0][1] = 0.0;
    vert[0][2] = 1.0*scale;

    vert[1][0] = 0.0;
    vert[1][1] = 0.0;
    vert[1][2] = 0.0;

    // Colours
    colour[0][0] = 0.0;
    colour[0][1] = 0.0;
    colour[0][2] = 1.0;
    colour[0][3] = 1.0;

    colour[1][0] = 0.0;
    colour[1][1] = 0.0;
    colour[1][2] = 1.0;
    colour[1][3] = 1.0;

    AddLine( vert, colour );

    End();
}

void lsBatch_Lines::buildGrid()
{
    Vector3f vert[2];
    Vector4f colour[2];

    Begin( 40 );

    for( int x=-4; x<5; x++ )
    {
        // X direction
        // Line verts
        vert[0][0] = -4.0;
        vert[0][1] = 0.0;
        vert[0][2] = x;

        vert[1][0] = 4.0;
        vert[1][1] = 0.0;
        vert[1][2] = x;

        // Colours
        colour[0][0] = 1.0;
        colour[0][1] = 0.0;
        colour[0][2] = 0.0;
        colour[0][3] = 1.0;

        colour[1][0] = 1.0;
        colour[1][1] = 0.0;
        colour[1][2] = 0.0;
        colour[1][3] = 1.0;

        AddLine( vert, colour );

        // Z direction
        // Line verts
        vert[0][0] = x;
        vert[0][1] = 0.0;
        vert[0][2] = -4.0;

        vert[1][0] = x;
        vert[1][1] = 0.0;
        vert[1][2] = 4.0;

        // Colours
        colour[0][0] = 0.0;
        colour[0][1] = 0.0;
        colour[0][2] = 1.0;
        colour[0][3] = 1.0;

        colour[1][0] = 0.0;
        colour[1][1] = 0.0;
        colour[1][2] = 1.0;
        colour[1][3] = 1.0;

        AddLine( vert, colour );
    }

    End();
}

void lsBatch_Lines::Copy_GLdata()
{
    // Create the buffer objects
    getOgl()->glGenBuffers( 1, &vbo_verts );
    getOgl()->glGenBuffers( 1, &vbo_colours );

    // Vertex data
    getOgl()->glBindBuffer( GL_ARRAY_BUFFER, vbo_verts );
    getOgl()->glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat)*nNumVerts*3, pVerts, GL_STATIC_DRAW );

    // Colours
    getOgl()->glBindBuffer( GL_ARRAY_BUFFER, vbo_colours );
    getOgl()->glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat)*nNumVerts*4, pColours, GL_STATIC_DRAW );
}

void lsBatch_Lines::Clear_GLdata()
{
    // Delete buffer objects
    getOgl()->glDeleteBuffers( 1, &vbo_colours );
    getOgl()->glDeleteBuffers( 1, &vbo_verts );
}

