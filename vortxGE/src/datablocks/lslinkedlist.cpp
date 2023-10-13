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

#include "lslistItem.h"
#include "lslistitem_vertexdata.h"

#include "lslinkedlist.h"

lsLinkedList::lsLinkedList()
{
    item_list = NULL;
    curr_item = NULL;
    item_count = 0;
}

lsLinkedList::~lsLinkedList()
{
    if( item_list ) delete item_list;
}

void lsLinkedList::Add_item(lsListItem *item)
{
    // item index is set by the user.
    item->setNext_item( item_list );
    item_list = item;
    item_count++;

    //For add at end func
    if( curr_item == NULL ) curr_item = item;
}

void lsLinkedList::Add_itematEnd(lsListItem *item)
{
    if(  curr_item == NULL )
    {
        Add_item( item );

    } else
    {
        curr_item->setNext_item( item );
        curr_item = item;
        item_count++;
    }
}

lsListItem *lsLinkedList::Find_item(int index)
{
    lsListItem *li = item_list;
    while( li )
    {
        if( li->getIndex() == index ) return li;
        li = li->getNext_item();
    }
    return li;
}

void lsLinkedList::Clear()
{
    if( item_list ) delete item_list;
    item_list = NULL;
    curr_item = NULL;
    item_count = 0;
}

/*!
 * \brief lsLinkedList::Add_ChkDupli
 *
 * Checks for duplicates, if found then deletes the instance.\n
 *
 * \param item
 * \return
 * Index of the item added or found.\n
 *
 */
int lsLinkedList::Add_ChkDupli( lsListItem_vertexdata *item )
{
    lsListItem_vertexdata *li = (lsListItem_vertexdata*)item_list;
    while( li )
    {
        if( li->isEqual( item ) )
        {
            // already exisits so delete and return current index
            delete item;
            return li->getIndex();
        }
        li = (lsListItem_vertexdata*)li->getNext_item();
    }
    // Add item
    Add_itematEnd( item );
    // Special case, we need to set the index
    // index_count was incremented in Add_item() so its fine to use as index.
    item->setIndex( item_count );
    return item->getIndex();
}

void lsLinkedList::Debug()
{
    int itype;

    lsListItem *li = item_list;
    if( li )
    {
        itype = li->getItem_type();
        if( li->getItem_type() == LSIT_INDEX )
        {
            cout << "Index list : ";
        }

        while( li )
        {
            if( li->getItem_type() == LSIT_INDEX )
            {
                cout << li->Debug();
                cout << " ";
            } else
            {
                cout << "Item index : " << li->getIndex();
                cout << " Data : " << li->Debug();
                cout << endl;
            }
            li = li->getNext_item();
        } // while( li )

        if( itype == LSIT_INDEX )
        {
            cout << endl;
        }
    } // if li
}

int lsLinkedList::getItem_count() const
{
    return item_count;
}

lsListItem *lsLinkedList::getItem_list() const
{
    return item_list;
}

