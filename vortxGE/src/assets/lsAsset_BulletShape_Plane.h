#ifndef vortxGE_lsAsset_BulletShape_Plane_h
#define vortxGE_lsAsset_BulletShape_Plane_h

// math
#include "../math/lsVector3.h"

#include "lsAsset.h"

/**
 * @brief Creates a bullet physics infinite plane
 *
 */
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


#endif
