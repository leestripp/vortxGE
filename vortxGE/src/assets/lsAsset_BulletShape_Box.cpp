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

#include "lsAsset_BulletShape_Box.h"

/**
 * @brief Stores the type of asset and stores the size.
 *
 * This does not create the bullet shape, you must call AllocAsset() before use.
 *
 * @param x ...
 * @param y ...
 * @param z ...
 * @param opengl ...
 */
lsAsset_BulletShape_Box::lsAsset_BulletShape_Box( float x, float y, float z ) : lsAsset(NULL)
{
    setAsset_type( LSAT_BULLETSHAPE_BOX );
    box = lsVector3( x, y, z );
}

lsAsset_BulletShape_Box::~lsAsset_BulletShape_Box()
{
    FreeAsset();
}


/**
 * @brief Creates the bullet shape ready for use.
 *
 * @return void
 */
void lsAsset_BulletShape_Box::AllocAsset()
{
    lsBulletShape *bulletshape;

    // make sure we dont create a duplicate
    FreeAsset();

    bulletshape = new lsBulletShape( getOpenGL() );
    if( bulletshape )
    {
        bulletshape->Make_Box( box.x, box.y, box.z );
        setData( bulletshape );
    } else
    {
        cout << "ERROR: Failed to allocate asset : " << getAsset_name() << endl;
    }
}

/**
 * @brief Destroy's the shape and removes it from memory.
 *
 * @return void
 */
void lsAsset_BulletShape_Box::FreeAsset()
{
    if( getData() )
    {
        delete getData();
        setData( NULL );
    }
}

