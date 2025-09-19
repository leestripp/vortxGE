#pragma once

// math
#include "../math/lsVector3.h"

#include "lsAsset.h"


class lsAsset_BulletShape_Plane : public lsAsset
{
public:
    lsAsset_BulletShape_Plane( float vectorX, float vectorY, float vectorZ );
    virtual ~lsAsset_BulletShape_Plane();

    // lsAsset
    void AllocAsset();
    void FreeAsset();

private:
    lsVector3 normal;

};
