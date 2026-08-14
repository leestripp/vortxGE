#include <iostream>
#include <cstring>

#include "../vortx_debug.h"

// render
#include "../render/lsRender.h"
// shader
#include "../shader/lsShader.h"

// scene
#include "../scene/lsObject.h"

#include "lsBatch_Lines.h"


lsBatch_Lines::lsBatch_Lines( lsRender *render ) : lsBatch(render)
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


void lsBatch_Lines::Begin( lsUInt nMaxVerts )
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

void lsBatch_Lines::Draw( lsCamera *cam, lsObject *object, int pass_type )
{
#ifdef VORTX_DEBUG_RENDER
    cout << "lsBatch_Lines::Draw - " << object->getName() << endl;
#endif

    if( getShader() )
    {
        getShader()->UseShader( cam, object, pass_type, this );
    }else
    {
        cout << "lsBatch_Lines::Draw - Shader missing" << endl;
    }

    // VAO
    getRender()->BindVertexArray( m_Vao );

    getRender()->DrawArrays( LS_LINES, 0, nNumVerts );

    if( getShader() ) getRender()->UseProgram( 0 );

#ifdef VORTX_DEBUG_OPENGLERRORS
    getRender()->CheckGLErrors( getName() );
#endif

    // Pass it on
    if( getNext_batch() ) getNext_batch()->Draw( cam, object, pass_type );
}

void lsBatch_Lines::Update( double delta )
{
    // Pass it on
    if( getNext_batch() ) getNext_batch()->Update( delta );
}

void lsBatch_Lines::buildEmpty(lsFloat scale)
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
    getRender()->GenVertexArrays( 1, &m_Vao );
    // Create the buffer objects
    getRender()->GenBuffers( 1, &vbo_verts );
    getRender()->GenBuffers( 1, &vbo_colours );

    // VAO start
    getRender()->BindVertexArray( m_Vao );

    // Vertex data
    getRender()->BindBuffer( LS_ARRAY_BUFFER, vbo_verts );
    getRender()->EnableVertexAttribArray( 0 ); // vxVertex
    getRender()->VertexAttribPointer( 0, 3, LS_FLOAT, LS_FALSE, 0, 0 );
    getRender()->BufferData( LS_ARRAY_BUFFER, sizeof(lsFloat)*nNumVerts*3, pVerts, LS_STATIC_DRAW );

    // Colours
    getRender()->BindBuffer( LS_ARRAY_BUFFER, vbo_colours );
    getRender()->EnableVertexAttribArray( 1 ); // vxColour
    getRender()->VertexAttribPointer( 1, 4, LS_FLOAT, LS_FALSE, 0, 0 );
    getRender()->BufferData( LS_ARRAY_BUFFER, sizeof(lsFloat)*nNumVerts*4, pColours, LS_STATIC_DRAW );

    // VAO end
    getRender()->BindVertexArray( 0 );
}

void lsBatch_Lines::Clear_GLdata()
{
    // Delete buffer objects
    getRender()->DeleteBuffers( 1, &vbo_colours );
    getRender()->DeleteBuffers( 1, &vbo_verts );
    // Delete VAO
    getRender()->DeleteVertexArrays( 1, &m_Vao );
}

