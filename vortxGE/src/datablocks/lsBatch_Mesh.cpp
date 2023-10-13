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

#include <cstring>

#include "../vortx_debug.h"

// Shaders
#include "../shader/lsShader.h"

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
    diffuse = lsVector4( 0.8, 0.8, 0.8, 1.0 );
    specular = lsVector4( 1.0, 1.0, 1.0, 1.0 );
    ambiant = lsVector4( 0.2, 0.2, 0.2, 1.0 );
    shininess = 20.0;
    transparent = 1.0;
    glow = 1.0;

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
    lsProperty_Vector4 *pv4;
    lsProperty_Float *pv1;

    pv4 = new lsProperty_Vector4( &diffuse );
    pv4->setProp_name( "Diffuse" );
    pv4->setProp_id( LSPROPID_DIFFUSE );
    Add_Property( pv4 );

    pv4 = new lsProperty_Vector4( &specular );
    pv4->setProp_name( "Specular" );
    pv4->setProp_id( LSPROPID_SPECULAR );
    Add_Property( pv4 );

    pv4 = new lsProperty_Vector4( &ambiant );
    pv4->setProp_name( "Ambient" );
    pv4->setProp_id( LSPROPID_AMBIENT );
    Add_Property( pv4 );

    pv1 = new lsProperty_Float( &shininess );
    pv1->setProp_name( "Shininess" );
    pv1->setProp_id( LSPROPID_SHININESS );
    Add_Property( pv1 );

    pv1 = new lsProperty_Float( &glow );
    pv1->setProp_name( "Glow" );
    pv1->setProp_id( LSPROPID_GLOW );
    Add_Property( pv1 );
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
    cout << "lsBatch_Mesh::Draw - Start" << endl;
#endif

    GLuint vertex, uv, tangent, normal;

    vertex = 0;
    uv = 0;
    tangent = 0;
    normal = 0;

    if( getShader() )
    {
        getShader()->UseShader( cam, object, pass, this );

        // Lights
        getShader()->Setup_Lights( cam, object );

        vertex = getOgl()->glGetAttribLocation( getShader()->getProgram(), "vxVertex" );
        uv = getOgl()->glGetAttribLocation( getShader()->getProgram(), "vxTexCoord" );
        tangent = getOgl()->glGetAttribLocation( getShader()->getProgram(), "vxTangent" );
        normal = getOgl()->glGetAttribLocation( getShader()->getProgram(), "vxNormal" );

    } else
    {
        cout << "lsBatch_Mesh::Draw - Shader missing" << endl;
    }

    // Verts
    getOgl()->glBindBuffer( GL_ARRAY_BUFFER, bufferObjects[ VERTEX_DATA ] );
    getOgl()->glEnableVertexAttribArray( vertex );
    getOgl()->glVertexAttribPointer( vertex, 3, GL_FLOAT, GL_FALSE, 0, 0 );

    // TexCoords
    getOgl()->glBindBuffer( GL_ARRAY_BUFFER, bufferObjects[ TEXTURE_DATA ] );
    getOgl()->glEnableVertexAttribArray( uv );
    getOgl()->glVertexAttribPointer( uv, 2, GL_FLOAT, GL_FALSE, 0, 0 );

    // Tangents
    getOgl()->glBindBuffer( GL_ARRAY_BUFFER, bufferObjects[ TANGENT_DATA ] );
    getOgl()->glEnableVertexAttribArray( tangent );
    getOgl()->glVertexAttribPointer( tangent, 3, GL_FLOAT, GL_FALSE, 0, 0 );

    // Normals
    getOgl()->glBindBuffer( GL_ARRAY_BUFFER, bufferObjects[ NORMAL_DATA ] );
    getOgl()->glEnableVertexAttribArray( normal );
    getOgl()->glVertexAttribPointer( normal, 3, GL_FLOAT, GL_FALSE, 0, 0 );

    // Draw Triangles
    getOgl()->glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, bufferObjects[ INDEX_DATA ] );
    glDrawElements( GL_TRIANGLES, count_tris, GL_UNSIGNED_SHORT, 0 );

    getOgl()->glDisableVertexAttribArray( vertex );
    getOgl()->glDisableVertexAttribArray( uv );
    getOgl()->glDisableVertexAttribArray( tangent );
    getOgl()->glDisableVertexAttribArray( normal );

    if( getShader() ) getOgl()->glUseProgram(0);

#ifdef VORTX_DEBUG_OPENGLERRORS
    cout << "Check GL errors : " << getName() << endl;
    getOgl()->CheckGLErrors();
#endif // VORTX_DEBUG_OPENGLERRORS

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
    // Create the buffer objects
    getOgl()->glGenBuffers( 6, bufferObjects );

    // Vertex data
    getOgl()->glBindBuffer( GL_ARRAY_BUFFER, bufferObjects[VERTEX_DATA] );
    getOgl()->glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat)*count_verts*3, pVerts, GL_STATIC_DRAW );
	addMem_used( sizeof(GLfloat)*count_verts*3 );

    // Normal data
    getOgl()->glBindBuffer( GL_ARRAY_BUFFER, bufferObjects[NORMAL_DATA] );
    getOgl()->glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat)*count_verts*3, pNormals, GL_STATIC_DRAW );
	addMem_used( sizeof(GLfloat)*count_verts*3 );

    // Tangent data
    getOgl()->glBindBuffer( GL_ARRAY_BUFFER, bufferObjects[TANGENT_DATA] );
    getOgl()->glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat)*count_verts*3, pTangents, GL_STATIC_DRAW );
	addMem_used( sizeof(GLfloat)*count_verts*3 );

    // Texture coordinates
    getOgl()->glBindBuffer( GL_ARRAY_BUFFER, bufferObjects[TEXTURE_DATA] );
    getOgl()->glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat)*count_verts*2, pTexCoords, GL_STATIC_DRAW );
	addMem_used( sizeof(GLfloat)*count_verts*2 );

    // Copy data
    getOgl()->glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, bufferObjects[INDEX_DATA] );
    getOgl()->glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*count_tris, pIndex, GL_STATIC_DRAW );
	addMem_used( sizeof(GLushort)*count_tris );

}

void lsBatch_Mesh::Clear_GLdata()
{
    // Delete buffer objects
    getOgl()->glDeleteBuffers( 6, bufferObjects );

	getOgl()->getStats()->addUsed_mem( -getMem_used() );
	setMem_used( 0 );
}

void lsBatch_Mesh::Add_Property( lsProperty* p )
{
    p->setNext_prop( property_list );
    if( property_list ) property_list->setPrev_prop( p );
    property_list = p;
}

