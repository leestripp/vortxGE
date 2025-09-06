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
