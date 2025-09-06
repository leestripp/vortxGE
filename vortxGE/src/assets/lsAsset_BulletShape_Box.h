#ifndef vortxGE_lsAsset_BulletShape_Box_h
#define vortxGE_lsAsset_BulletShape_Box_h

// math
#include "../math/lsVector3.h"

#include "lsAsset.h"


/**
 * @brief Creates a bullet physics box shape.
 *
 */
class lsAsset_BulletShape_Box : public lsAsset
{
public:
    lsAsset_BulletShape_Box( float x, float y, float z );
    virtual ~lsAsset_BulletShape_Box();

    // lsAsset
    void AllocAsset();
    void FreeAsset();

private:
    lsVector3 box;

};


#endif
