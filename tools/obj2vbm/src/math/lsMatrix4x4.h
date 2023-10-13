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

#ifndef LSMATRIX4X4_H_
#define LSMATRIX4X4_H_

#include <iostream>
#include <string>

class lsVector3;
class lsQuaternion;

using namespace std;

//! Matrix Math
/*!
 * 4x4 Matrix calculations.
 *
 */
class lsMatrix4x4
{
public:
	lsMatrix4x4();
	lsMatrix4x4(const float mat4[16]);
	lsMatrix4x4(const lsQuaternion &q, float x=0, float y=0, float z=0);
	virtual ~lsMatrix4x4();

	lsVector3 Rotate(const lsVector3 &v);
	void Set(const lsQuaternion &q, float x=0, float y=0, float z=0);
	void LoadIdentity();

	// Transformations
	void RotateMatrix( float angle, float x, float y, float z );
	void TranslateMatrix( float x, float y, float z );
	void ScaleMatrix( float x, float y, float z );

	// Utilities
	void PerspectiveProjection( float fovy, float aspect_ratio, float near_plane, float far_plane );
	void OrthographicProjection( float xMin, float xMax, float yMin, float yMax, float zMin, float zMax );

	float Cotangent(float angle);
	void LookAt( const lsVector3& eyePosition, const lsVector3& lookAt, const lsVector3& upVector );

	lsMatrix4x4& operator=(const lsMatrix4x4& m);
	lsMatrix4x4& operator*=( const lsMatrix4x4& B );
	lsMatrix4x4 operator*( const lsMatrix4x4& B );
	lsVector3 operator*(const lsVector3 &v);

	// Debug
	void debug( const string& str );


	// Data
	float matrix[16];

};

#endif /* LSMATRIX4X4_H_ */
