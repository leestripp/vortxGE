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

#include <iostream>
// debug
#include "../vortx_debug.h"

#include "lslistitem_facetri.h"

lsListItem_faceTri::lsListItem_faceTri() : lsListItem()
{
    setItem_type( LSIT_FACETRI );
}

string lsListItem_faceTri::Debug()
{
    stringstream ss;

    ss << "face quad( ";
    // V1
    ss << "(" << v1;
    ss << "/" << n1;
    ss << "/" << uv1;
    ss << ") ";
    // V2
    ss << "(" << v2;
    ss << "/" << n2;
    ss << "/" << uv2;
    ss << ") ";
    // V3
    ss << "(" << v3;
    ss << "/" << n3;
    ss << "/" << uv3;
    ss << ") )";

    return ss.str();
}

