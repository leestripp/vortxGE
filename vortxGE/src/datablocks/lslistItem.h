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

#ifndef LSLISTITEM_H
#define LSLISTITEM_H

#include <iostream>
#include <string>
#include <sstream>

const int LSIT_NONE         = 0;
const int LSIT_VECTOR2      = 1;
const int LSIT_VECTOR3      = 2;
const int LSIT_VECTOR4      = 3;
const int LSIT_INDEX        = 4;        // List of unsigned short's
const int LSIT_VERTEXDATA   = 5;        // All vertex data per index : vertex, normal, tangent and uv
const int LSIT_FACETRI      = 6;        // Index data for a triangle
const int LSIT_FACEQUAD     = 7;        // Index data for a Quad

using namespace std;

/*!
 * \brief The lsList class
 *
 * A simple linked list class. Subclass it to add your own data.\n
 * \n
 *
 */
class lsListItem
{
public:
    lsListItem();
    virtual ~lsListItem();

    lsListItem *getNext_item() const;
    void setNext_item(lsListItem *value);

    int getItem_type() const;
    void setItem_type(int value);

    int getIndex() const;
    void setIndex(int value);

    // debug
    virtual string Debug() = 0;

private:
    lsListItem *next_item;
    int item_type;
    int index;
};

#endif // LSLISTITEM_H
