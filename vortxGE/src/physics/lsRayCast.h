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

#ifndef LSRAYCAST_H
#define LSRAYCAST_H

#include <iostream>
#include <string>

#include <bullet/btBulletDynamicsCommon.h>


//! Bullet Physics Ray
/*!
 * Cast a ray into the scene, point A to B.
 *
 * */
class lsRayCast
{
    public:
        lsRayCast( btDiscreteDynamicsWorld* dw );
        virtual ~lsRayCast();

        void castRay( const btVector3& from,const btVector3& to );

    private:
        btDiscreteDynamicsWorld* world;

        btVector3 hit_pos;
        btVector3 hit_norm;

};

#endif // LSRAYCAST_H
