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

#include "lsVector2.h"

lsVector2::lsVector2()
{
	x = 0.0f;
	y = 0.0f;
}

lsVector2::lsVector2( float v1, float v2 )
{
	x = v1;
	y = v2;
}

lsVector2::~lsVector2()
{
}


lsVector2 lsVector2::operator+(const lsVector2 &v)
{
	lsVector2 r;

	r.x = x + v.x;
	r.y = y + v.y;

	return r;
}

lsVector2 lsVector2::operator-(const lsVector2 &v)
{
	lsVector2 r;

	r.x = x - v.x;
	r.y = y - v.y;

	return r;
}


lsVector2 lsVector2::operator*(const lsVector2 &v)
{
	lsVector2 r;

	r.x = x * v.x;
	r.y = y * v.y;

	return r;
}

lsVector2 lsVector2::operator*(float t)
{
	lsVector2 r;

	r.x = x * t;
	r.y = y * t;

	return r;
}

lsVector2 lsVector2::operator/(float t)
{
	lsVector2 r;

	r.x = x / t;
	r.y = y / t;

	return r;
}


