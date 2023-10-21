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

#ifndef LSBATCH_MESH_H
#define LSBATCH_MESH_H

// math
#include "../math/lsVector2.h"
#include "../math/lsVector3.h"
#include "../math/lsVector4.h"

#include "lsBatch.h"

// Buffer IDs
const int VERTEX_DATA		= 0;
const int COLOUR_DATA		= 1;
const int NORMAL_DATA		= 2;
const int TANGENT_DATA		= 3;
const int UV_DATA		    = 4;
const int INDEX_DATA		= 5;

class lsOpenGL;
class lsObject;
class lsPass;
class lsCamera;
class lsProperty;
class lsAnimationManager;
class lsListItem_vertexdata;


class lsBatch_Mesh : public lsBatch
{
public:
    lsBatch_Mesh( lsOpenGL *opengl );
    virtual ~lsBatch_Mesh();

    // lsBatch
    void Draw( lsCamera *cam, lsObject *object, lsPass *pass );
    void Update( double delta );

    // Add data
    void Init_data( int vc, int fit );
    void Add_VertexData( lsListItem_vertexdata *vd );
    void Add_Index( int index );

    // openGL data - on video card : needed by the asset system
    void Copy_GLdata();
    void Clear_GLdata();

    // Animation Properties
    void Add_Property( lsProperty* p );

private:
    GLuint vao_mesh;
    GLuint bufferObjects[6];

    // Data
    Vector3f *pVerts;
    Vector3f *pNormals;
    Vector3f *pTangents;
    Vector2f *pTexCoords;
    // Index data
    GLushort *pIndex;
    // index counter for addind data
    int vertex_index;
    int tri_index;
    // counts for copy GL data
    int count_verts;
    int count_tris;

    // Animation
    lsAnimationManager *animationmanager;
    // Properties
    lsProperty *property_list;

public:
    // Material
    lsVector4 diffuse;
    lsVector4 specular;
    lsVector4 ambiant;
    GLfloat shininess;
    GLfloat transparent;
    GLfloat glow;
    // Textures
    GLuint tex_diffuse;
    GLuint tex_normal;
    GLuint tex_specular;

};

#endif // LSBATCH_MESH_H
