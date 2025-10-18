#include "../datablocks/lsBulletShape.h"
#include "lsAsset_BulletShape_Pill.h"
#include <iostream>

lsAsset_BulletShape_Pill::lsAsset_BulletShape_Pill(float radius, float height)
    : lsAsset(nullptr), m_radius(radius), m_height(height)
{
    setAsset_type(LSAT_BULLETSHAPE_PILL);
}

lsAsset_BulletShape_Pill::~lsAsset_BulletShape_Pill()
{
    FreeAsset();
}

void lsAsset_BulletShape_Pill::AllocAsset()
{
    lsBulletShape *bulletshape;
	
	// Free old asset.
    FreeAsset();
	
    bulletshape = new lsBulletShape(getOpenGL());
    if(bulletshape)
    {
        bulletshape->Make_Pill(m_radius, m_height);
        setData(bulletshape);
    } else
    {
        cout << "ERROR: Failed to allocate asset : " << getAsset_name() << endl;
    }
}

void lsAsset_BulletShape_Pill::FreeAsset()
{
    if (getData())
    {
        delete getData();
        setData(nullptr);
    }
}
