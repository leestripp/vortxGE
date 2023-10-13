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

#include "lsMath.h"

#include "lsVector2.h"
#include "lsVector3.h"


lsMath::lsMath()
{

}

lsMath::~lsMath()
{
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
