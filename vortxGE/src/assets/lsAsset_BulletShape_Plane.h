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
