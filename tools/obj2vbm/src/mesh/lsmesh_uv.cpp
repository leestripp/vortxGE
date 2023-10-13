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

#include "lsmesh_uv.h"

lsMesh_UV::lsMesh_UV()
{
    next_uv = NULL;
    uv_index = 0;
}

lsMesh_UV::~lsMesh_UV()
{
    if( next_uv ) delete next_uv;
}

void lsMesh_UV::Set_UV(float u, float v)
{
    uv[0] = u;
    uv[1] = v;
}

float *lsMesh_UV::Get_UV()
{
    return uv;
}

lsMesh_UV *lsMesh_UV::getNext_uv() const
{
    return next_uv;
}

void lsMesh_UV::setNext_uv(lsMesh_UV *value)
{
    next_uv = value;
}

int lsMesh_UV::getUv_index() const
{
    return uv_index;
}

void lsMesh_UV::setUv_index(int value)
{
    uv_index = value;
}

string lsMesh_UV::toString()
{
    stringstream ss;
    ss << uv_index << " " << uv[0] << " " << uv[1];
    return ss.str();
}

