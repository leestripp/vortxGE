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

#ifndef LSQUATERNION_H_
#define LSQUATERNION_H_

#define DELTA 1e-10     // error tolerance

//! Quaternion
/*!
 * Basiclly this is just a 3D vector with a roll angle added.
 * Avoids Gimbal Lock problems.
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
