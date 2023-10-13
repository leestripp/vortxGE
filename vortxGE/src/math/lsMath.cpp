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

#include "lsMath.h"

#include "lsVector2.h"
#include "lsVector3.h"


lsMath::lsMath()
{

}

lsMath::~lsMath()
{
}

bool lsMath::approxEqual( float a, float b, float epsilon )
{
    return fabs(a - b) <= ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}


lsVector3 lsMath::Cross(const lsVector3& Vec1, const lsVector3& Vec2)
{
	lsVector3 res;

	res.x = Vec1.y * Vec2.z - Vec1.z * Vec2.y;
	res.y = Vec1.z * Vec2.x - Vec1.x * Vec2.z;
	res.z = Vec1.x * Vec2.y - Vec1.y * Vec2.x;

	return res;
}

float lsMath::Dot( const lsVector3& Vec1, const lsVector3& Vec2 )
{
	float res;

	res = Vec1.x * Vec2.x + Vec1.y * Vec2.y + Vec1.z * Vec2.z;

	return res;
}
