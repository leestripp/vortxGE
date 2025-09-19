// vortxGE
#include "../vortxGE.h"

// datablocks
#include "../datablocks/lsBulletShape.h"

#include "lsAsset_BulletShape_Plane.h"

lsAsset_BulletShape_Plane::lsAsset_BulletShape_Plane( float vectorX, float vectorY, float vectorZ ) : lsAsset(NULL)
{
    setAsset_type( LSAT_BULLETSHAPE_PLANE );
    normal = lsVector3( vectorX, vectorY, vectorZ );
}

lsAsset_BulletShape_Plane::~lsAsset_BulletShape_Plane()
{
    FreeAsset();
}


// #######################
// AllocAsset

void lsAsset_BulletShape_Plane::AllocAsset()
{
    lsBulletShape *bulletshape;

    // clean up any old asset
    FreeAsset();

    bulletshape = new lsBulletShape( getOpenGL() );
    if( bulletshape )
    {
        bulletshape->Make_Plane( normal.x, normal.y, normal.z );

        setData( bulletshape );
    }
}

void lsAsset_BulletShape_Plane::FreeAsset()
{
    if( getData() )
    {
        delete getData();
        setData( NULL );
    }
}
