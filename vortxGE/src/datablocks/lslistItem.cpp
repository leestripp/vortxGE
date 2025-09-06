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

