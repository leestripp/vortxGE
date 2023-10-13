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

#ifndef LSLISTITEM_FACEQUAD_H
#define LSLISTITEM_FACEQUAD_H

#include "lslistItem.h"

class lsListItem_faceQuad : public lsListItem
{
public:
    lsListItem_faceQuad();

    // debug
    string Debug();

    // data
    // V1
    int v1;
    int n1;
    int uv1;
    // V2
    int v2;
    int n2;
    int uv2;
    // V3
    int v3;
    int n3;
    int uv3;
    // V4
    int v4;
    int n4;
    int uv4;
};

#endif // LSLISTITEM_FACEQUAD_H
