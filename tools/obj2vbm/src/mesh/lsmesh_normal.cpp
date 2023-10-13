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

#include <iostream>
#include <sstream>

// math
#include "../math/lsVector3.h"

#include "lsmesh_normal.h"


lsMesh_Normal::lsMesh_Normal()
{
    next_normal = NULL;
    nindex = 0;
    // Normal
    Set_Normal( 1, 0, 0 );
    // Tangent
    Set_Tangent( 0, 1, 0 );
    // Binormal
    Set_Binormal( 0, 0, 1 );
}

lsMesh_Normal::lsMesh_Normal( lsMesh_Normal *copy )
{
    next_normal = NULL;
    nindex = 0;

    float *N, *T, *B;
    N = copy->Get_Normal();
    T = copy->Get_Tangent();
    B = copy->Get_Binormal();
    // Normal
    Set_Normal( N[0], N[1], N[2] );
    // Tangent
    Set_Tangent( T[0], T[1], T[2] );
    // Binormal
    Set_Binormal( B[0], B[1], B[2] );
}

lsMesh_Normal::~lsMesh_Normal()
{
    if( next_normal ) delete next_normal;
}

void lsMesh_Normal::Set_Normal(float x, float y, float z)
{
    lsVector3 vec( x, y, z );
    vec.normalize();

    normal[0] = vec.getX();
    normal[1] = vec.getY();
    normal[2] = vec.getZ();
}

float *lsMesh_Normal::Get_Normal()
{
    return normal;
}

void lsMesh_Normal::Set_Tangent(float x, float y, float z)
{
    lsVector3 vec( x, y, z );
    vec.normalize();

    tangent[0] = vec.getX();
    tangent[1] = vec.getY();
    tangent[2] = vec.getZ();
}

float *lsMesh_Normal::Get_Tangent()
{
    return tangent;
}

void lsMesh_Normal::Set_Binormal(float x, float y, float z)
{
    lsVector3 vec( x, y, z );
    vec.normalize();

    binormal[0] = vec.getX();
    binormal[1] = vec.getY();
    binormal[2] = vec.getZ();
}

float *lsMesh_Normal::Get_Binormal()
{
    return binormal;
}


lsMesh_Normal *lsMesh_Normal::getNext_normal() const
{
    return next_normal;
}

void lsMesh_Normal::setNext_normal(lsMesh_Normal *value)
{
    next_normal = value;
}

int lsMesh_Normal::getNindex() const
{
    return nindex;
}

void lsMesh_Normal::setNindex(int value)
{
    nindex = value;
}

string lsMesh_Normal::toString()
{
    stringstream ss;
    ss << nindex << " " << normal[0] << " " << normal[1] << " " << normal[2];
    return ss.str();
}

string lsMesh_Normal::toStringTangent()
{
    stringstream ss;
    ss << nindex << " " << tangent[0] << " " << tangent[1] << " " << tangent[2];
    return ss.str();
}
