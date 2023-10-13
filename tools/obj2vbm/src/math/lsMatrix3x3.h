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

#ifndef LSMATRIX3X3_H
#define LSMATRIX3X3_H

#include <iostream>
#include <string>

#include "lsMatrix4x4.h"

using namespace std;

class lsMatrix3x3
{
public:
	lsMatrix3x3();
	virtual ~lsMatrix3x3();

	void ExtractRotationMatrix4x4( const lsMatrix4x4& mat );

	// debug
	void debug( const string& str );

	// data
	float matrix[9];

};

#endif // LSMATRIX3X3_H
