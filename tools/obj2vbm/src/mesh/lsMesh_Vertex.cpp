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

#include <sstream>

#include "lsMesh_Vertex.h"


lsMesh_Vertex::lsMesh_Vertex()
{
    next_vertex = 0;
    vindex = 0;

    // data
    vertex[0] = 0;
    vertex[1] = 0;
    vertex[2] = 0;
}

lsMesh_Vertex::~lsMesh_Vertex()
{
    if ( next_vertex ) delete next_vertex;
}


lsMesh_Vertex* lsMesh_Vertex::getNext_vertex()
{
    return next_vertex;
}

void lsMesh_Vertex::setNext_vertex( lsMesh_Vertex* v )
{
    next_vertex = v;
}

int lsMesh_Vertex::getIndex()
{
    return vindex;
}

void lsMesh_Vertex::setIndex( int i )
{
    vindex = i;
}

float* lsMesh_Vertex::getVertexData()
{
    return vertex;
}

void lsMesh_Vertex::setVertexData( float x, float y, float z )
{
    vertex[0] = x;
    vertex[1] = y;
    vertex[2] = z;
}

string lsMesh_Vertex::toString()
{
    stringstream ss;
    ss << vindex << " " << vertex[0] << " " << vertex[1] << " " << vertex[2];
    return ss.str();
}

