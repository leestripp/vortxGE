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

#ifndef LSMESH_VERTEX_H
#define LSMESH_VERTEX_H

#include <string>

using namespace std;

class lsMesh_Vertex
{
public:
    lsMesh_Vertex();
    virtual ~lsMesh_Vertex();

    // get and set
    lsMesh_Vertex* getNext_vertex();
    void setNext_vertex( lsMesh_Vertex* v );
    int getIndex();
    void setIndex( int i );
    float* getVertexData();
    void setVertexData( float x, float y, float z );

    // Save
    string toString();

private:
    lsMesh_Vertex* next_vertex;
    int vindex;

    // vertex
    float vertex[3];
};

#endif // LSMESH_VERTEX_H
