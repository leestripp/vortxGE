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

#ifndef LSQUATERNION_H_
#define LSQUATERNION_H_

#define DELTA 1e-10     // error tolerance

//! Quaternion
/*!
 * Not used, so the code was never finished.
 *
 */
class lsQuaternion
{
public:
	lsQuaternion();
	lsQuaternion(float w, float x, float y, float z);
	lsQuaternion(float roll, float pitch, float yaw);
	virtual ~lsQuaternion();

	lsQuaternion& operator*=( const lsQuaternion& q );
	lsQuaternion operator*( const lsQuaternion& q2 );

	void Set(float w, float x, float y, float z);
	void Slerp(const lsQuaternion &to, float t);
	void Slerp(const lsQuaternion &from, const lsQuaternion &to, float t);
	void Lerp(const lsQuaternion &to, float t);

	// Date
	float w, x, y, z;

};

#endif /* LSQUATERNION_H_ */
