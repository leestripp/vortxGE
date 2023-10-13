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

#ifndef LSLISTITEM_VECTOR3_H
#define LSLISTITEM_VECTOR3_H

// math
#include "../math/lsMath.h"

#include "lslistItem.h"

class lsListItem_Vector3 : public lsListItem
{
public:
    lsListItem_Vector3();

    // debug
    string Debug();

    // Data
    Vector3f vec;
};

#endif // LSLISTITEM_VECTOR3_H
