#include <iostream>
// debug
#include "../vortx_debug.h"

// opengl
#include "../opengl/lsOpenGL.h"

// shader
#include "../shader/lsShader.h"

// scene
#include "../scene/lsObject.h"

#include "lsBatch_Quad.h"

// Data ***********************************

GLfloat vertices[] = {
    1, 1, 0,
    -1,  1, 0,
    -1, -1, 0,
    1,  -1, 0
};

GLfloat uvs[] = {
    1, 1,
    0, 1,
    0, 0,
    1, 0
};

GLubyte indices[] = {
    0, 2, 1,
    0, 3, 2
};


// code ***********************************

lsBatch_Quad::lsBatch_Quad( lsOpenGL *opengl ) : lsBatch(opengl)
{
    setBatch_type( LSBT_QUAD );
    setName( "Batch Quad" );

    // Textures
    tex_diffuse = 0;

    Copy_GLdata();
}

lsBatch_Quad::~lsBatch_Quad()
{
    Clear_GLdata();
}

void lsBatch_Quad::Draw( lsCamera* cam, lsObject *object, lsPass *pass )
{
#ifdef VORTX_DEBUG_RENDER
    cout << "lsBatch_Quad::Draw - " << object->getName() << endl;
#endif

    if( getShader() )
    {
        getShader()->UseShader( cam, object, pass, this );

    } else
    {
        cout << "lsBatch_Quad::Draw - Shader missing" << endl;
    }

    // Bind VAO
    glBindVertexArray( vao_quad );

    glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0 );

    if( getShader() ) glUseProgram( 0 );

#ifdef VORTX_DEBUG_OPENGLERRORS
    getOgl()->CheckGLErrors( getName() );
#endif

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
    // VAO vao_quad
    glGenVertexArrays( 1, &vao_quad );

    // Create the buffer objects
    glGenBuffers( 1, &vbo_verts );
    glGenBuffers( 1, &vbo_uvs );
    glGenBuffers( 1, &vbo_indices );

    // VAO start
    glBindVertexArray( vao_quad );

    // Vertex data
    glBindBuffer( GL_ARRAY_BUFFER, vbo_verts );
    glEnableVertexAttribArray( 0 );  // vxVertex
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    glBufferData( GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW );

    // uv data
    glBindBuffer( GL_ARRAY_BUFFER, vbo_uvs );
    glEnableVertexAttribArray( 4 ); //vxTexCord
    glVertexAttribPointer( 4, 2, GL_FLOAT, GL_FALSE, 0, 0 );
    glBufferData( GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW );

    // Indexes
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbo_indices );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW );

    // VAO end
    glBindVertexArray( 0 );

}

void lsBatch_Quad::Clear_GLdata()
{
#if defined(VORTX_DEBUG_BATCH)
    cout << "lsBatch_Quad::~lsBatch_Quad : Free GL buffers " << getName() << endl;
#endif

    // Delete buffer objects
    glDeleteBuffers( 1, &vbo_uvs );
    glDeleteBuffers( 1, &vbo_verts );
    glDeleteBuffers( 1, &vbo_indices );
    // VAO vao_quad
    glDeleteVertexArrays( 1, &vao_quad );
}
