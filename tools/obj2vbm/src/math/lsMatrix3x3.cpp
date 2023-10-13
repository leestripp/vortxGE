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

#include "lsMatrix3x3.h"


lsMatrix3x3::lsMatrix3x3()
{
	// X
	matrix[0] = 1.0;
	matrix[1] = 0.0;
	matrix[2] = 0.0;
	// Y
	matrix[3] = 0.0;
	matrix[4] = 1.0;
	matrix[5] = 0.0;
	// Z
	matrix[6] = 0.0;
	matrix[7] = 0.0;
	matrix[8] = 1.0;
}

lsMatrix3x3::~lsMatrix3x3()
{
}

//! Extract a rotation matrix
/*!
 * Extract a rotation matrix from a 4x4 matrix
 *
 * */
void lsMatrix3x3::ExtractRotationMatrix4x4( const lsMatrix4x4& mat )
{
	// X
	matrix[0] = mat.matrix[ 0];
	matrix[1] = mat.matrix[ 1];
	matrix[2] = mat.matrix[ 2];
	// Y
	matrix[3] = mat.matrix[ 4];
	matrix[4] = mat.matrix[ 5];
	matrix[5] = mat.matrix[ 6];
	// Z
	matrix[6] = mat.matrix[ 8];
	matrix[7] = mat.matrix[ 9];
	matrix[8] = mat.matrix[10];
}

void lsMatrix3x3::debug( const string& str )
{

	cout << "Matrix : " << str << endl;

	int k;
	for( int i=0; i<3; i++ )
	{
		for( int j=0; j<9; j+=3 )
		{
			k = j+i;
			// cout << " m[" << k << "] = " << mat[k];
			cout << "[ " << matrix[k] << " ]   ";

		}
		cout << endl;
	}
}


