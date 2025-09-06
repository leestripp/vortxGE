#include <iostream>
// debug
#include "../vortx_debug.h"

#include "lslistitem_index.h"


lsListItem_index::lsListItem_index() : lsListItem()
{
    setItem_type( LSIT_INDEX );
}

string lsListItem_index::Debug()
{
    stringstream ss;

    ss << getIndex();

    return ss.str();
}
