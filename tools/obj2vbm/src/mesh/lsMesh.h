/*
 * Copyright (C) 2015 by Lee A. Stripp <leestripp@gmail.com>
 *
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contributor(s):
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#ifndef LSMESH_H
#define LSMESH_H

#include <iostream>
#include <string>

// filesystem
#include "../filesystem/lsFileIO.h"

// convert
#include "../convert/lsString.h"

// Math
#include "../math/lsVector3.h"
#include "../math/lsBoundingSphere.h"

class lsMesh_Vertex;
class lsMesh_Normal;
class lsMesh_UV;
class lsMesh_Face;
class lsMesh_FaceGroup;
class lsMaterial;

using namespace std;

//! Load VXM mesh
/*!
 *
 */
class lsMesh
{
public:
    lsMesh();
    virtual ~lsMesh();

    // Create Mesh
    bool Load_Mesh( const string& filename );
    void Load_Material( const string& filename );

    void Add_Vertex( int index, lsMesh_Vertex* vertex );
    void Add_Normal( int index, lsMesh_Normal* normal );
    void Add_UV( int index, lsMesh_UV* uv );
    void Add_FaceGroup( lsMesh_FaceGroup* group );
    void Add_Material( lsMaterial *material );

    // Find
    lsMesh_Vertex*  Find_Vertex( int index );
    lsMesh_Normal*  Find_Normal( int index );
    lsMesh_UV*      Find_UV( int index );
    lsMesh_Face* Find_Face( int index );
    lsMaterial* Find_Material( const string& name );

    // Data
    lsMesh_Vertex* getVertex_list();

    // Utilities
    float FindRadius();
    lsVector3 FindCenter();

    // Tangents
    void buildTangents();
    void computeTangentBasis( lsMesh_Normal *normal, lsMesh_Vertex* vertex1, lsMesh_Vertex* vertex2, lsMesh_Vertex* vertex3, lsMesh_UV* UV1, lsMesh_UV* UV2, lsMesh_UV* UV3 );

    void meshStats();

    // Save
    void exportVBM( const string& filename );

private:
    lsMesh_Vertex*      vertex_list;
    lsMesh_Normal*      normals_list;
    lsMesh_UV*          uv_list;
    lsMesh_FaceGroup*   group_list;
    lsMaterial*         material_list;
    lsBoundingSphere    bSphere;

    // counters
    int vertex_count;
    int normal_count;
    int uv_count;

};

#endif // LSMESH_H
