/*
 * Copyright (C) 2015 by Lee A. Stripp <leestripp@gmail.com>
 *
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contributor(s):
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#ifndef LSPLANE_H_
#define LSPLANE_H_

#include <iostream>

#include "lsVector3.h"

using namespace std;

//! Plane math class
/*!
 * Holds a point and normal vector
 *
 */
class lsPlane
{
public:
	lsPlane();
	lsPlane( lsVector3 &v1, lsVector3 &v2, lsVector3 &v3 );
	virtual ~lsPlane();


	void set3Points( lsVector3 &v1,  lsVector3 &v2,  lsVector3 &v3);
	void setNormalAndPoint( const lsVector3 &normal, const lsVector3 &point);
	void setCoefficients( float a, float b, float c, float d );
	float distance(const lsVector3 &p );

	void debug();


	lsVector3 normal;
	lsVector3 point;
	float d;

};

#endif /* LSPLANE_H_ */
