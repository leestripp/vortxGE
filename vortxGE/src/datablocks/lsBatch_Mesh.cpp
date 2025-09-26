#include <iostream>
// debug
#include "../vortx_debug.h"

// Shaders
#include "../shader/lsShaderManager.h"
#include "../shader/lsShader.h"

// passes
#include "../passes/lsPass.h"
#include "../passes/lsPass_ShadowMap.h"

// Scene
#include "../scene/lsNode.h"
#include "../scene/lsObject.h"
#include "../scene/lsLight.h"

// datablocks
#include "lslistitem_vertexdata.h"

// Properties
#include "../properties/lsProperty.h"
#include "../properties/lsProperty_Vector3.h"
#include "../properties/lsProperty_Vector4.h"
#include "../properties/lsProperty_Float.h"

// animation
#include "../animation/lsAnimationManager.h"

// stats
#include "lsStats.h"

#include "lsBatch_Mesh.h"


lsBatch_Mesh::lsBatch_Mesh( lsOpenGL *opengl ) : lsBatch(opengl)
{
	setBatch_type( LSBT_MESH );
	setName( "Batch: Mesh" );

	// data
	pVerts = NULL;
	pNormals = NULL;
	pTangents = NULL;
	pTexCoords = NULL;
	// Index data
	pIndex = NULL;

	// counters - used for adding data
	vertex_index = 0;
	tri_index = 0;
	count_tris = 0;
	
	// Material
	m_albedo = lsVector4(1.0, 1.0, 1.0, 1.0);
	m_roughness = 0.5;
	m_metallic = 0.0;
	
	// Create animation manager
	animationmanager = new lsAnimationManager();
	if(! animationmanager )
	{
		// ERROR
		cout << "lsActor::lsActor - ERROR: cant create lsAnimationManager instance" << endl;
	}

	// properties
	property_list = NULL;

	// Add default properties
	// lsProperty_Vector4 *pv4;
	// lsProperty_Float *pv1;
	
	// TODO: Add new PBR properties.
	// Material.
	
}

lsBatch_Mesh::~lsBatch_Mesh()
{
	// Delete GL buffers
	Clear_GLdata();

	// DO NOT delete textures they are assets.

	// Cleanup
	if( pIndex ) delete [] pIndex;
	if( pVerts ) delete [] pVerts;
	if( pNormals ) delete [] pNormals;
	if( pTangents ) delete [] pTangents;
	if( pTexCoords ) delete [] pTexCoords;

	// self cleanning
	if( property_list ) delete property_list;
	if( animationmanager ) delete animationmanager;
}

void lsBatch_Mesh::Draw( lsCamera *cam, lsObject *object, lsPass *pass )
{
#ifdef VORTX_DEBUG_RENDER
	cout << "lsBatch_Mesh::Draw - START: " << object->getName() << endl;
#endif

	lsShader* shaderToUse = getShader();
	if (pass->getPass_type() == LSPT_SHADOWMAP) {
		shaderToUse = getOgl()->getShaderManager()->Find_Shader("shadowmap");
	}

	if( shaderToUse )
	{
		shaderToUse->UseShader( cam, object, pass, this );

		// Lights only for non-shadow passes
		if (pass->getPass_type() != LSPT_SHADOWMAP)
		{
			shaderToUse->Setup_Lights( cam, object );
		}

	} else
	{
		cout << "lsBatch_Mesh::Draw - Shader missing" << endl;
	}

	// Bind VAO vao_mesh
	glBindVertexArray( vao_mesh );

	glDrawElements( GL_TRIANGLES, count_tris, GL_UNSIGNED_SHORT, 0 );

	if( shaderToUse ) glUseProgram(0);

#ifdef VORTX_DEBUG_RENDER
	cout << "lsBatch_Mesh::Draw - END: " << object->getName() << endl;
#endif

	// Pass it on
	if( getNext_batch() ) getNext_batch()->Draw( cam, object, pass );
}

void lsBatch_Mesh::Update( double delta )
{
	// Materials can be animated by default
	if( animationmanager )
	{
		animationmanager->UpdateAll( delta );
	}
}

void lsBatch_Mesh::Init_data( int vc, int fit )
{
	count_verts = vc;
	count_tris = fit;

	// vector3f
	pVerts = new Vector3f[vc];
	pNormals = new Vector3f[vc];
	pTangents = new Vector3f[vc];
	// Uvs
	pTexCoords = new Vector2f[vc];

	// Index data
	pIndex = new GLushort[ fit ];

}

void lsBatch_Mesh::Add_VertexData(lsListItem_vertexdata *vd)
{
	// Vertex
	pVerts[ vertex_index ][0] = vd->vertex[0];
	pVerts[ vertex_index ][1] = vd->vertex[1];
	pVerts[ vertex_index ][2] = vd->vertex[2];
	// normal
	pNormals[ vertex_index ][0] = vd->normal[0];
	pNormals[ vertex_index ][1] = vd->normal[1];
	pNormals[ vertex_index ][2] = vd->normal[2];
	// tangent
	pTangents[ vertex_index ][0] = vd->tangent[0];
	pTangents[ vertex_index ][1] = vd->tangent[1];
	pTangents[ vertex_index ][2] = vd->tangent[2];
	// uv
	pTexCoords[ vertex_index ][0] = vd->uv[0];
	pTexCoords[ vertex_index ][1] = vd->uv[1];

	vertex_index++;
	// lsBatch
	setVertex_count( getVertex_count() + 1 );
}

void lsBatch_Mesh::Add_Index( int index )
{
	pIndex[ tri_index ] = index-1;
	tri_index++;
}

void lsBatch_Mesh::Copy_GLdata()
{
	// create buffers
	glGenVertexArrays( 1, &vao_mesh );

	/* mesh shader code ref
			layout (location = 0) in vec4 vxVertex;
			layout (location = 1) in vec4 vxColour;
			layout (location = 2) in vec3 vxNormal;
			layout (location = 3) in vec3 vxTangent;
			layout (location = 4) in vec2 vxTexCoord;
	*/
	glGenBuffers( 6, bufferObjects ); // buffer 6 is for index data

	// VAO start
	glBindVertexArray( vao_mesh );

	// Vertex data
	glBindBuffer( GL_ARRAY_BUFFER, bufferObjects[VERTEX_DATA] );
	glEnableVertexAttribArray( 0 ); // vxVertex
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
	glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat)*count_verts*3, pVerts, GL_STATIC_DRAW );
	addMem_used( sizeof(GLfloat)*count_verts*3 );

	// Normal data
	glBindBuffer( GL_ARRAY_BUFFER, bufferObjects[NORMAL_DATA] );
	glEnableVertexAttribArray( 2 );
	glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, 0 );
	glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat)*count_verts*3, pNormals, GL_STATIC_DRAW );
	addMem_used( sizeof(GLfloat)*count_verts*3 );

	// Tangent data
	glBindBuffer( GL_ARRAY_BUFFER, bufferObjects[TANGENT_DATA] );
	glEnableVertexAttribArray( 3 );
	glVertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, 0, 0 );
	glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat)*count_verts*3, pTangents, GL_STATIC_DRAW );
	addMem_used( sizeof(GLfloat)*count_verts*3 );

	// Texture coordinates
	glBindBuffer( GL_ARRAY_BUFFER, bufferObjects[UV_DATA] );
	glEnableVertexAttribArray( 4 );
	glVertexAttribPointer( 4, 2, GL_FLOAT, GL_FALSE, 0, 0 );
	glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat)*count_verts*2, pTexCoords, GL_STATIC_DRAW );
	addMem_used( sizeof(GLfloat)*count_verts*2 );

	// index data
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, bufferObjects[INDEX_DATA] );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*count_tris, pIndex, GL_STATIC_DRAW );
	addMem_used( sizeof(GLushort)*count_tris );

	// VAO end
	glBindVertexArray( 0 );

}

void lsBatch_Mesh::Clear_GLdata()
{
	// Delete buffer objects
	glDeleteBuffers( 6, bufferObjects );
	glDeleteVertexArrays( 1, &vao_mesh );

	getOgl()->getStats()->addUsed_mem( -getMem_used() );
	setMem_used( 0 );
}

void lsBatch_Mesh::Add_Property( lsProperty* p )
{
	p->setNext_prop( property_list );
	if( property_list ) property_list->setPrev_prop( p );
	property_list = p;
}

