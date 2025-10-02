#include <iostream>
// debug
#include "../vortx_debug.h"

#include "lsBatch_Normals.h"

lsBatch_Normals::lsBatch_Normals( lsRender *render ) : lsBatch_Lines(render)
{
    setBatch_type( LSBT_NORMALS );
    setName("Batch Normals");
}

lsBatch_Normals::~lsBatch_Normals()
{
}
