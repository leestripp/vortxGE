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

#include <cmath>

#include "lsQuaternion.h"


lsQuaternion::lsQuaternion()
{
	w = 1.0;
	x = 0;
	y = 0;
	z = 0;
}

lsQuaternion::lsQuaternion(float w, float x, float y, float z)
{
	this->w = w;
	this->x = x;
	this->y = y;
	this->z = z;
}

lsQuaternion::lsQuaternion(float roll, float pitch, float yaw)
{
	float cr = cos(roll*0.5);
	float cp = cos(pitch*0.5);
	float cy = cos(yaw*0.5);

	float sr = sin(roll/2);
	float sp = sin(pitch/2);
	float sy = sin(yaw/2);

	float cpcy = cp * cy;
	float spsy = sp * sy;
	float cpsy = cp * sy;
	float spcy = sp * cy;

	w = cr * cpcy + sr * spsy;
	x = sr * cpcy - cr * spsy;
	y = cr * spcy + sr * cpsy;
	z = cr * cpsy - sr * spcy;
}

lsQuaternion::~lsQuaternion()
{
}



lsQuaternion& lsQuaternion::operator*=(const lsQuaternion& q)
{
	float E, F, G, H;

	float tw = w;
	float tx = x;
	float ty = y;

	E = (tx +  z)*(q.x + q.y);
	F = (tx -  z)*(q.x - q.y);
	G = (tw + ty)*(q.w - q.z);
	H = (tw - ty)*(q.w + q.z);

	w = ( z - ty)*(q.y - q.z) + (-E - F + G + H)/2;
	x = (tw + tx)*(q.w + q.x) - ( E + F + G + H)/2;
	y = (tw - tx)*(q.y + q.z) + ( E - F + G - H)/2;
	z = (ty +  z)*(q.w - q.x) + ( E - F - G + H)/2;

	return *this;
}

lsQuaternion lsQuaternion::operator*(const lsQuaternion& q2)
{
	lsQuaternion qr;

	float E, F, G, H;

	E = (x + z)*(q2.x + q2.y);
	F = (x - z)*(q2.x - q2.y);
	G = (w + y)*(q2.w - q2.z);
	H = (w - y)*(q2.w + q2.z);

	qr.w = (z - y)*(q2.y - q2.z) + (-E - F + G + H)*0.5;
	qr.x = (w + x)*(q2.w + q2.x) - ( E + F + G + H)*0.5;
	qr.y = (w - x)*(q2.y + q2.z) + ( E - F + G - H)*0.5;
	qr.z = (y + z)*(q2.w - q2.x) + ( E - F - G + H)*0.5;

	return qr;
}

void lsQuaternion::Set(float w, float x, float y, float z)
{
	this->w = w;
	this->x = x;
	this->y = y;
	this->z = z;
}

/*!
 * slerp(p,q,t) = (p*sin((1-t)*omega) + q*sin(t*omega)) / sin(omega)
 *
 * */
void lsQuaternion::Slerp(const lsQuaternion& to, float t)
{
	float to1[4];
	float omega, cosom, sinom;
	float scale0, scale1;

	// calc cosine
	cosom = x*to.x + y*to.y + z*to.z + w*to.w;

	// adjust signs (if necessary)
	if (cosom < 0.0)
	{
		cosom = -cosom;
		to1[0] = - to.x;
		to1[1] = - to.y;
		to1[2] = - to.z;
		to1[3] = - to.w;
	} else  {
		to1[0] = to.x;
		to1[1] = to.y;
		to1[2] = to.z;
		to1[3] = to.w;
	}

	// calculate coefficients
	if ((1.0 - cosom) > DELTA)
	{
		// standard case (slerp)
		omega = acos(cosom);
		sinom = sin(omega);
		scale0 = (float)sin((1.0 - t) * omega) / sinom;
		scale1 = (float)sin(t * omega) / sinom;
	} else {
		// "from" and "to" quaternions are very close
		//  ... so we can do a linear interpolation
		scale0 = 1.0f - t;
		scale1 = t;
	}

	// calculate final values
	x = scale0*x + scale1*to1[0];
	y = scale0*y + scale1*to1[1];
	z = scale0*z + scale1*to1[2];
	w = scale0*w + scale1*to1[3];
}

void lsQuaternion::Slerp(const lsQuaternion& from, const lsQuaternion& to, float t)
{
	float to1[4];
	float omega, cosom, sinom;
	float scale0, scale1;

	// calc cosine
	cosom = from.x*to.x + from.y*to.y + from.z*to.z + from.w*to.w;

	// adjust signs (if necessary)
	if (cosom < 0.0)
	{
		cosom = -cosom;
		to1[0] = - to.x;
		to1[1] = - to.y;
		to1[2] = - to.z;
		to1[3] = - to.w;
	} else
	{
		to1[0] = to.x;
		to1[1] = to.y;
		to1[2] = to.z;
		to1[3] = to.w;
	}

	// calculate coefficients
	if ((1.0 - cosom) > DELTA)
	{
		// standard case (slerp)
		omega = acos(cosom);
		sinom = sin(omega);
		scale0 = (float)sin((1.0 - t) * omega) / sinom;
		scale1 = (float)sin(t * omega) / sinom;
	} else
	{
		// "from" and "to" quaternions are very close
		//  ... so we can do a linear interpolation
		scale0 = 1.0f - t;
		scale1 = t;
	}

	// calculate final values
	x = scale0*from.x + scale1*to1[0];
	y = scale0*from.y + scale1*to1[1];
	z = scale0*from.z + scale1*to1[2];
	w = scale0*from.w + scale1*to1[3];
}


/*!
 * Linearly interpolates between two quaternion positions
 * f ast but not as nearly as smooth as Slerp
 *
 * */
void lsQuaternion::Lerp(const lsQuaternion& to, float t)
{
	float to1[4];
	float cosom;
	float scale0, scale1;

	// calc cosine
	cosom = x*to.x + y*to.y + z*to.z + w*to.w;

	// adjust signs (if necessary)
	if( cosom < 0.0 )
	{
		to1[0] = - to.x;
		to1[1] = - to.y;
		to1[2] = - to.z;
		to1[3] = - to.w;
	} else
	{
		to1[0] = to.x;
		to1[1] = to.y;
		to1[2] = to.z;
		to1[3] = to.w;
	}

	// interpolate linearly
	scale0 = 1.0f - t;
	scale1 = t;

	// calculate final values
	x = scale0*x + scale1*to1[0];
	y = scale0*y + scale1*to1[1];
	z = scale0*z + scale1*to1[2];
	w = scale0*w + scale1*to1[3];
}


