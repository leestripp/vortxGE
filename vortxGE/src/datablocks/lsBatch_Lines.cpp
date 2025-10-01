#include <iostream>
#include <cstring>

#include "../vortx_debug.h"

// opengl
#include "../render/lsOpenGL.h"

// shader
#include "../shader/lsShader.h"

// scene
#include "../scene/lsObject.h"

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

// Remove all base CPU side data
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
    cout << "lsBatch_Lines::Draw - " << object->getName() << endl;
#endif

    if( getShader() )
    {
        getShader()->UseShader( cam, object, pass, this );
    }else
    {
        cout << "lsBatch_Lines::Draw - Shader missing" << endl;
    }

    // VAO
    glBindVertexArray(m_Vao);

    glDrawArrays( GL_LINES, 0, nNumVerts );

    if( getShader() ) glUseProgram(0);

#ifdef VORTX_DEBUG_OPENGLERRORS
    getOgl()->CheckGLErrors( getName() );
#endif

    // Pass it on
    if( getNext_batch() ) getNext_batch()->Draw( cam, object, pass );
}

void lsBatch_Lines::Update( double delta )
{
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
    // VAO
    glGenVertexArrays(1, &m_Vao );
    // Create the buffer objects
    glGenBuffers( 1, &vbo_verts );
    glGenBuffers( 1, &vbo_colours );

    // VAO start
    glBindVertexArray( m_Vao );

    // Vertex data
    glBindBuffer( GL_ARRAY_BUFFER, vbo_verts );
    glEnableVertexAttribArray( 0 ); // vxVertex
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat)*nNumVerts*3, pVerts, GL_STATIC_DRAW );

    // Colours
    glBindBuffer( GL_ARRAY_BUFFER, vbo_colours );
    glEnableVertexAttribArray( 1 ); // vxColour
    glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, 0, 0 );
    glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat)*nNumVerts*4, pColours, GL_STATIC_DRAW );

    // VAO end
    glBindVertexArray( 0 );
}

void lsBatch_Lines::Clear_GLdata()
{
    // Delete buffer objects
    glDeleteBuffers( 1, &vbo_colours );
    glDeleteBuffers( 1, &vbo_verts );
    // Delete VAO
    glDeleteVertexArrays( 1, &m_Vao );
}

