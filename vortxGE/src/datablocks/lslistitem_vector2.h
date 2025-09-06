#ifndef LSLISTITEM_VECTOR2_H
#define LSLISTITEM_VECTOR2_H

// math
#include "../math/lsMath.h"

#include "lslistItem.h"


class lsListItem_Vector2 : public lsListItem
{
public:
    lsListItem_Vector2();

    // debug
    string Debug();

    // data
    Vector2f vec;
};

#endif // LSLISTITEM_VECTOR2_H
