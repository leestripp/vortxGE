#include <iostream>
// debug
#include "../vortx_debug.h"

// shader
#include "../shader/lsShader.h"

// scene
#include "../scene/lsObject.h"

#include "lsBatch_Quad.h"

// Data ***********************************

lsFloat vertices[] = {
	1, 1, 0,
	-1,  1, 0,
	-1, -1, 0,
	1,  -1, 0
};

lsFloat uvs[] = {
	0, 1,
	1, 1,
	1, 0,
	0, 0
};

lsUByte indices[] = {
	0, 2, 1,
	0, 3, 2
};


// code ***********************************

lsBatch_Quad::lsBatch_Quad( lsRender *render ) : lsBatch(render)
{
	setBatch_type( LSBT_QUAD );
	setName( "Batch Quad" );
	
	// Textures
	tex_albedo = 0;
	
	Copy_GLdata();
}

lsBatch_Quad::~lsBatch_Quad()
{
	Clear_GLdata();
}

void lsBatch_Quad::Draw( lsCamera* cam, lsObject *object, int pass_type )
{
#ifdef VORTX_DEBUG_RENDER
	cout << "lsBatch_Quad::Draw - " << object->getName() << endl;
#endif

	if( getShader() )
	{
		getShader()->UseShader( cam, object, pass_type, this );

	} else
	{
		cout << "lsBatch_Quad::Draw - Shader missing" << endl;
	}

	// Bind VAO
	getRender()->BindVertexArray( vao_quad );

	getRender()->DrawElements( LS_TRIANGLES, 6, LS_UNSIGNED_BYTE, 0 );

	if( getShader() ) getRender()->UseProgram( 0 );

#ifdef VORTX_DEBUG_OPENGLERRORS
	getRender()->CheckGLErrors( getName() );
#endif

	// Pass it on
	if( getNext_batch() ) getNext_batch()->Draw( cam, object, pass_type );
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
	getRender()->GenVertexArrays( 1, &vao_quad );

	// Create the buffer objects
	getRender()->GenBuffers( 1, &vbo_verts );
	getRender()->GenBuffers( 1, &vbo_uvs );
	getRender()->GenBuffers( 1, &vbo_indices );

	// VAO start
	getRender()->BindVertexArray( vao_quad );

	// Vertex data
	getRender()->BindBuffer( LS_ARRAY_BUFFER, vbo_verts );
	getRender()->EnableVertexAttribArray( 0 );  // vxVertex
	getRender()->VertexAttribPointer( 0, 3, LS_FLOAT, LS_FALSE, 0, 0 );
	getRender()->BufferData( LS_ARRAY_BUFFER, sizeof(vertices), vertices, LS_STATIC_DRAW );

	// uv data
	getRender()->BindBuffer( LS_ARRAY_BUFFER, vbo_uvs );
	getRender()->EnableVertexAttribArray( 4 ); //vxTexCord
	getRender()->VertexAttribPointer( 4, 2, LS_FLOAT, LS_FALSE, 0, 0 );
	getRender()->BufferData( LS_ARRAY_BUFFER, sizeof(uvs), uvs, LS_STATIC_DRAW );

	// Indexes
	getRender()->BindBuffer( LS_ELEMENT_ARRAY_BUFFER, vbo_indices );
	getRender()->BufferData( LS_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, LS_STATIC_DRAW );

	// VAO end
	getRender()->BindVertexArray( 0 );

}

void lsBatch_Quad::Clear_GLdata()
{
#if defined(VORTX_DEBUG_BATCH)
	cout << "lsBatch_Quad::~lsBatch_Quad : Free GL buffers " << getName() << endl;
#endif

	// Delete buffer objects
	getRender()->DeleteBuffers( 1, &vbo_uvs );
	getRender()->DeleteBuffers( 1, &vbo_verts );
	getRender()->DeleteBuffers( 1, &vbo_indices );
	// VAO vao_quad
	getRender()->DeleteVertexArrays( 1, &vao_quad );
}
