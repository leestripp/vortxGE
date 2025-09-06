#include <iostream>
// debug
#include "../vortx_debug.h"

#include "lslistitem_facequad.h"

lsListItem_faceQuad::lsListItem_faceQuad() : lsListItem()
{
    setItem_type( LSIT_FACEQUAD );
}

string lsListItem_faceQuad::Debug()
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
    ss << ") ";
    // V4
    ss << "(" << v4;
    ss << "/" << n4;
    ss << "/" << uv4;
    ss << ") )";

    return ss.str();
}

