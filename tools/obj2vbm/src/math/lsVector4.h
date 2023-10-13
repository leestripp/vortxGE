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

#ifndef LSVECTOR4_H_
#define LSVECTOR4_H_

#include <iostream>
#include <cmath>

using namespace std;

//! Position, Vectors
/*!
 * 3D vector class with functions.
 * \n
 * x, y, z, w
 */
class lsVector4
{
public:
	lsVector4();
	lsVector4( float XX, float YY, float ZZ, float WW );
	lsVector4( const lsVector4 &v );
	virtual ~lsVector4();

	lsVector4 operator+(float s);
	lsVector4 operator+(const lsVector4 &v);
	lsVector4 operator-(const lsVector4 &v);
	lsVector4 operator*(const lsVector4 &v);
	lsVector4 operator*(float t);
	lsVector4 operator/(float t);
	lsVector4 operator-();

	float length();
	void normalize();

	void copy(const lsVector4 &v);
	void set(float x,float y, float z, float w );
	float distance( const lsVector4 &V1 );

    void debug(const string& msg);

	float getX() const
	{
		return x;
	}

	void setX(float x)
	{
		this->x = x;
	}

	float getY() const
	{
		return y;
	}

	void setY(float y)
	{
		this->y = y;
	}

	float getZ() const
	{
		return z;
	}

	void setZ(float z)
	{
		this->z = z;
	}

	float getW() const
	{
		return w;
	}

	void setW(float w)
	{
		this->w = w;
	}


	float x;
	float y;
	float z;
	float w;

};

#endif /* LSVECTOR3_H_ */
