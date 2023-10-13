/*
 * Copyright (c) 2016 by Lee A. Stripp <leestripp@gmail.com>
 *
 * This file is part of the vortxGE library.
 *
 * vortxGE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * vortxGE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with vortxGE.  If not, see <http://www.gnu.org/licenses/>
 */

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
// lsAsset

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

