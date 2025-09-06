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
