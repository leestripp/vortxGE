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

#ifndef LSMESH_NORMAL_H
#define LSMESH_NORMAL_H

#include <string>

using namespace std;

class lsMesh_Normal
{
public:
    lsMesh_Normal();
    lsMesh_Normal( lsMesh_Normal *copy );
    virtual ~lsMesh_Normal();

    // Data
    void Set_Normal( float x, float y, float z );
    float* Get_Normal();
    void Set_Tangent( float x, float y, float z );
    float* Get_Tangent();
    void Set_Binormal( float x, float y, float z );
    float* Get_Binormal();

    // get set
    lsMesh_Normal *getNext_normal() const;
    void setNext_normal(lsMesh_Normal *value);
    int getNindex() const;
    void setNindex(int value);

    // Save
    string toString();
    string toStringTangent();

private:
    lsMesh_Normal* next_normal;
    int nindex;
    // Data
    float normal[3];
    float tangent[3];
    float binormal[3];
};

#endif // LSMESH_NORMAL_H
