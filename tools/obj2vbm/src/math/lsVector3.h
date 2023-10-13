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

#ifndef LSVECTOR3_H_
#define LSVECTOR3_H_

#include <iostream>
#include <cmath>

using namespace std;

//! Position, Vectors
/*!
 * 3D vector class with functions.
 *
 */
class lsVector3
{
public:
	lsVector3();
	lsVector3( float XX, float YY, float ZZ );
	lsVector3( const lsVector3 &v );
	virtual ~lsVector3();

	lsVector3& operator=(const lsVector3 &v);
	lsVector3& operator+=(const lsVector3 &v);
	lsVector3& operator+=(float s);
	lsVector3& operator-=(const lsVector3 &v);
	lsVector3& operator-=(float s);
	lsVector3& operator*=(const lsVector3 &v);
	lsVector3& operator*=(float s);
	lsVector3& operator/=(float s);

	lsVector3 operator+(float s);
	lsVector3 operator+(const lsVector3 &v);
	lsVector3 operator-(float s);
	lsVector3 operator-(const lsVector3 &v);
	lsVector3 operator*(float t);
	lsVector3 operator*(const lsVector3 &v);
	lsVector3 operator/(float t);

	lsVector3& RotateAboutX(float angle);
	lsVector3& RotateAboutY(float angle);
	lsVector3& RotateAboutZ(float angle);
	lsVector3& RotateAboutAxis(float angle, const lsVector3& axis);

	void set( const lsVector3& val );
	float length();
	lsVector3& normalize();

	void copy(const lsVector3 &v);
	void set(float x,float y, float z);

	void add( const lsVector3& vec );
	lsVector3 scalarMult(float a);
	float distance( const lsVector3 &V1 );

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


	float x;
	float y;
	float z;

};

#endif /* LSVECTOR3_H_ */
