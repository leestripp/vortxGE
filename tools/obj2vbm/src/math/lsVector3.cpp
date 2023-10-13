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

#include "lsVector3.h"


lsVector3::lsVector3()
{
	x = 0;
	y = 0;
	z = 0;
}

lsVector3::lsVector3( float XX, float YY, float ZZ )
{
	x = XX;
	y = YY;
	z = ZZ;
}

lsVector3::lsVector3( const lsVector3 &v )
{
	x = v.x;
	y = v.y;
	z = v.z;

}

lsVector3::~lsVector3()
{
}


// ***************
// ***** Methods
// ***************

lsVector3& lsVector3::operator=(const lsVector3 &v)
{
	// Same object?
	if (this == &v)
		return *this;

	x = v.x;
	y = v.y;
	z = v.z;

	return *this;
}

lsVector3& lsVector3::operator+=(const lsVector3 &v)
{
	// Same object?
	if (this == &v)
		return *this;

	x += v.x;
	y += v.y;
	z += v.z;

	return *this;
}

lsVector3& lsVector3::operator+=(float s)
{
	x += s;
	y += s;
	z += s;

	return *this;
}


lsVector3& lsVector3::operator-=(const lsVector3 &v)
{
	// Same object?
	if (this == &v)
		return *this;

	x -= v.x;
	y -= v.y;
	z -= v.z;

	return *this;
}

lsVector3& lsVector3::operator-=(float s)
{
	x -= s;
	y -= s;
	z -= s;

	return *this;
}

lsVector3& lsVector3::operator*=(const lsVector3 &v)
{
	x = y * v.z - z * v.y;
	y = z * v.x - x * v.z;
	z = x * v.y - y * v.x;

	return *this;
}

lsVector3& lsVector3::operator*=(float t)
{
	x *= t;
	y *= t;
	z *= t;

	return *this;
}

lsVector3& lsVector3::operator/=(float t)
{
	float f = 1.0 / t;
	x *= f;
	y *= f;
	z *= f;

	return *this;
}





lsVector3 lsVector3::operator+(float s)
{
	lsVector3 result;

	result.x = x + s;
	result.y = y + s;
	result.z = z + s;

	return result;
}

lsVector3 lsVector3::operator+(const lsVector3 &v)
{
	lsVector3 result;

	result.x = x + v.x;
	result.y = y + v.y;
	result.z = z + v.z;

	return result;
}

lsVector3 lsVector3::operator-(float s)
{
	lsVector3 result;

	result.x = x - s;
	result.y = y - s;
	result.z = z - s;

	return result;
}

lsVector3 lsVector3::operator-(const lsVector3 &v)
{
	lsVector3 result;

	result.x = x - v.x;
	result.y = y - v.y;
	result.z = z - v.z;

	return result;
}

lsVector3 lsVector3::operator*(float t)
{
	lsVector3 result;

	result.x = x * t;
	result.y = y * t;
	result.z = z * t;

	return result;
}

lsVector3 lsVector3::operator*(const lsVector3 &v)
{
	lsVector3 result;

	result.x = y * v.z - z * v.y;
	result.y = z * v.x - x * v.z;
	result.z = x * v.y - y * v.x;

	return result;
}

lsVector3 lsVector3::operator/(float t)
{
	lsVector3 result;

	float f = 1.0 / t;
	result.x = x * f;
	result.y = y * f;
	result.z = z * f;

	return result;
}

lsVector3& lsVector3::RotateAboutX(float angle)
{
	float s = sin(angle);
	float c = cos(angle);

	float ny = c * y - s * z;
	float nz = c * z + s * y;

	y = ny;
	z = nz;

	return *this;
}

lsVector3& lsVector3::RotateAboutY(float angle)
{
	float s = sin(angle);
	float c = cos(angle);

	float nx = c * x + s * z;
	float nz = c * z - s * x;

	x = nx;
	z = nz;

	return *this;
}

lsVector3& lsVector3::RotateAboutZ(float angle)
{
	float s = sin(angle);
	float c = cos(angle);

	float nx = c * x - s * y;
	float ny = c * y + s * x;

	x = nx;
	y = ny;

	return *this;
}

lsVector3& lsVector3::RotateAboutAxis(float angle, const lsVector3& axis)
{
	float s = sin(angle);
	float c = cos(angle);
	float k = 1.0 - c;

	float nx = x * (c + k * axis.x * axis.x) + y * (k * axis.x * axis.y - s * axis.z)
	+ z * (k * axis.x * axis.z + s * axis.y);
	float ny = x * (k * axis.x * axis.y + s * axis.z) + y * (c + k * axis.y * axis.y) + z * (k * axis.y * axis.z - s * axis.x);
	float nz = x * (k * axis.x * axis.z - s * axis.y) + y * (k * axis.y * axis.z + s * axis.x) + z * (c + k * axis.z * axis.z);

	x = nx;
	y = ny;
	z = nz;

	return *this;
}







float lsVector3::length()
{
    float l;

    l = (x*x) + (y*y) + (z*z);

    return( (float)sqrt( l ) );
}

lsVector3& lsVector3::normalize()
{
	return (*this /= sqrt(x * x + y * y + z * z));
}

void lsVector3::copy(const lsVector3 &v)
{
	x = v.x;
	y = v.y;
	z = v.z;
}

void lsVector3::set(float x,float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

void lsVector3::set( const lsVector3& val )
{
	this->x = val.x;
	this->y = val.y;
	this->z = val.z;
}

void lsVector3::add(const lsVector3& vec)
{
	x += vec.x;
	y += vec.y;
	z += vec.z;

}


lsVector3 lsVector3::scalarMult(float a)
{
	lsVector3 res;

	res.x = x * a;
	res.y = y * a;
	res.z = z * a;

	return res;
}

float lsVector3::distance( const lsVector3 &V1 )
{
	float dist;
	lsVector3 V3;
	lsVector3 t(x,y,z);

	V3 = t - V1;
	dist = sqrt( V3.x*V3.x + V3.y*V3.y + V3.z*V3.z );

	return dist;
}

// ***************
// ***** Debug
// ***************

void lsVector3::debug(const string& msg)
{
    cout << msg;
	cout << " Vector(" << x << "," << y << "," << z << ") " << endl;
}


