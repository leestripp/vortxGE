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

#include "lsEvent_HitPoint.h"


lsEvent_HitPoint::lsEvent_HitPoint()
{
	setEvent_type( LSET_HIT_POINT );

	hit_node = NULL;

}

lsEvent_HitPoint::~lsEvent_HitPoint()
{
}

void lsEvent_HitPoint::Set_HitNode(lsNode* node)
{
	hit_node = node;
}

void lsEvent_HitPoint::Set_HitNormal(float x, float y, float z)
{
	hit_normal.x = x;
	hit_normal.y = y;
	hit_normal.z = z;
}

void lsEvent_HitPoint::Set_HitPoint(float x, float y, float z)
{
	hit_point.x = x;
	hit_point.y = y;
	hit_point.z = z;
}
