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

#ifndef LSVECTOR2_H_
#define LSVECTOR2_H_

//! 2D Point
/*!
 * Storage and math functions
 *
 */
class lsVector2
{
public:
	lsVector2();
	lsVector2( float v1, float v2 );
	virtual ~lsVector2();

	lsVector2 operator+(const lsVector2 &v);
	lsVector2 operator-(const lsVector2 &v);
	lsVector2 operator*(const lsVector2 &v);
	lsVector2 operator*(float t);
	lsVector2 operator/(float t);


	float x;
	float y;

};

#endif /* LSVECTOR2_H_ */
