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

#include "lslistitem_vector3.h"


lsListItem_Vector3::lsListItem_Vector3() : lsListItem()
{
    setItem_type( LSIT_VECTOR3 );
}

string lsListItem_Vector3::Debug()
{
    stringstream ss;

    ss << "vector3f( " << vec[0];
    ss << ", " << vec[1];
    ss << ", " << vec[2];
    ss << " )";

    return ss.str();
}
