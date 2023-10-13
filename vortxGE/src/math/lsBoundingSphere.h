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

#ifndef LSBOUNDINGSPHERE_H_
#define LSBOUNDINGSPHERE_H_

#include "lsVector3.h"


//! Simple Bounding Sphere
/*!
 * Used for culling, Physics, collision etc...
 *
 */
class lsBoundingSphere
{
public:
	lsBoundingSphere();
	virtual ~lsBoundingSphere();

	float distance( const lsVector3& V1 );

	void draw();

	// Data
	lsVector3	center;
	float		radius;

};

#endif /* LSBOUNDINGSPHERE_H_ */
