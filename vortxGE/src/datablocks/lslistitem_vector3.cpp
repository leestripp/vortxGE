#include "lslistitem_vector3.h"


lsListItem_Vector3::lsListItem_Vector3() : lsListItem()
{
    setItem_type( LSIT_VECTOR3 );
}

string lsListItem_Vector3::Debug()
{
    stringstream ss;

    ss << "vector3f( " << vec[0];
    ss << ", " << vec[1];
    ss << ", " << vec[2];
    ss << " )";

    return ss.str();
}
