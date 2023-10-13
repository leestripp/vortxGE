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

#ifndef LSMESH_UV_H
#define LSMESH_UV_H

#include <string>

using namespace std;

class lsMesh_UV
{
public:
    lsMesh_UV();
    virtual ~lsMesh_UV();

    // Data
    void Set_UV( float u, float v );
    float *Get_UV();

    // get set
    lsMesh_UV *getNext_uv() const;
    void setNext_uv(lsMesh_UV *value);
    int getUv_index() const;
    void setUv_index(int value);

    // Save
    string toString();

private:
    lsMesh_UV *next_uv;
    int uv_index;

    // data
    float uv[2];
};

#endif // LSMESH_UV_H
