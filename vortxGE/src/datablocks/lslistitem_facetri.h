#ifndef LSLISTITEM_FACETRI_H
#define LSLISTITEM_FACETRI_H

#include "lslistItem.h"


class lsListItem_faceTri : public lsListItem
{
public:
    lsListItem_faceTri();

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
};

#endif // LSLISTITEM_FACETRI_H
