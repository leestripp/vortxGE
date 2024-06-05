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

#ifndef LSVECTOR4_H_
#define LSVECTOR4_H_

#include <iostream>
#include <sstream>
#include <string>
#include <cmath>

// array types
typedef float	Vector4f[4];


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

    const string& toString();
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
	
private:
    string output;
};

#endif /* LSVECTOR3_H_ */
