// math
#include "../math/lsMath.h"

#include "lslistitem_vertexdata.h"

lsListItem_vertexdata::lsListItem_vertexdata() : lsListItem()
{
    setItem_type( LSIT_VERTEXDATA );
}

bool lsListItem_vertexdata::isEqual( lsListItem_vertexdata *val )
{
    lsMath math;
    float e = 0.0001; // epsilon
    // vertex
    if(! math.approxEqual( vertex[0], val->vertex[0], e) ) return false;
    if(! math.approxEqual( vertex[1], val->vertex[1], e) ) return false;
    if(! math.approxEqual( vertex[2], val->vertex[2], e) ) return false;

    // normal
    if(! math.approxEqual( normal[0], val->normal[0], e) ) return false;
    if(! math.approxEqual( normal[1], val->normal[1], e) ) return false;
    if(! math.approxEqual( normal[2], val->normal[2], e) ) return false;

    // normal
    if(! math.approxEqual( tangent[0], val->tangent[0], e) ) return false;
    if(! math.approxEqual( tangent[1], val->tangent[1], e) ) return false;
    if(! math.approxEqual( tangent[2], val->tangent[2], e) ) return false;

    // uv
    if(! math.approxEqual( uv[0], val->uv[0], e) ) return false;
    if(! math.approxEqual( uv[1], val->uv[1], e) ) return false;

    return true;
}

string lsListItem_vertexdata::Debug()
{
    stringstream ss;

    ss << "vertex( " << vertex[0];
    ss << ", " << vertex[1];
    ss << ", " << vertex[2];
    ss << " )";

    ss << ",  normal( " << normal[0];
    ss << ", " << normal[1];
    ss << ", " << normal[2];
    ss << " )";

    return ss.str();
}
