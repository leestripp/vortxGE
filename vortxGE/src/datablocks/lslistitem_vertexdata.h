#ifndef LSLISTITEM_VERTEXDATA_H
#define LSLISTITEM_VERTEXDATA_H

// math
#include "../math/lsMath.h"

#include "lslistItem.h"

class lsListItem_vertexdata : public lsListItem
{
public:
    lsListItem_vertexdata();

    bool isEqual( lsListItem_vertexdata *val );

    // debug
    string Debug();

    // data
    Vector3f vertex;
    Vector3f normal;
    Vector3f tangent;
    Vector2f uv;
};

#endif // LSLISTITEM_VERTEXDATA_H
