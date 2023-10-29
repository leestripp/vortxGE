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

#include "lslistItem.h"

lsListItem::lsListItem()
{
    next_item = NULL;
    index = 0;

    item_type = LSIT_NONE;
}

lsListItem::~lsListItem()
{
    if( next_item ) delete next_item;
}

lsListItem *lsListItem::getNext_item() const
{
    return next_item;
}

void lsListItem::setNext_item(lsListItem *value)
{
    next_item = value;
}

int lsListItem::getItem_type() const
{
    return item_type;
}

void lsListItem::setItem_type(int value)
{
    item_type = value;
}

int lsListItem::getIndex() const
{
    return index;
}

void lsListItem::setIndex(int value)
{
    index = value;
}

