#include <iostream>
// debug
#include "../vortx_debug.h"

#include "lslistitem_facetri.h"

lsListItem_faceTri::lsListItem_faceTri() : lsListItem()
{
    setItem_type( LSIT_FACETRI );
}

string lsListItem_faceTri::Debug()
{
    stringstream ss;

    ss << "face quad( ";
    // V1
    ss << "(" << v1;
    ss << "/" << n1;
    ss << "/" << uv1;
    ss << ") ";
    // V2
    ss << "(" << v2;
    ss << "/" << n2;
    ss << "/" << uv2;
    ss << ") ";
    // V3
    ss << "(" << v3;
    ss << "/" << n3;
    ss << "/" << uv3;
    ss << ") )";

    return ss.str();
}

