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

#ifndef LSVECTOR2_H_
#define LSVECTOR2_H_

#include <sstream>
#include <string>

// array types
typedef float	Vector2f[2];

using namespace std;

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
	
	lsVector2& operator=( const lsVector2& other );
	lsVector2& operator+(const lsVector2 &v);
	lsVector2& operator-(const lsVector2 &v);
	lsVector2& operator*(const lsVector2 &v);
	lsVector2& operator*(float t);
	lsVector2& operator/(float t);
	
    const string& toString();

	float x;
	float y;

private:
    string output;

};

#endif /* LSVECTOR2_H_ */
