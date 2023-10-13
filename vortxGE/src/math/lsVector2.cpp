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

#include "lsVector2.h"


lsVector2::lsVector2()
{
	x = 0.0;
	y = 0.0;
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


const string& lsVector2::toString()
{
	stringstream ss;

	ss << "(";
	ss << x;
	ss << ",";
	ss << y;
	ss << ")";
	output = ss.str();

	return output;
}

