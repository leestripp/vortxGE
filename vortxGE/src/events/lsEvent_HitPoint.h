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

#ifndef LSEVENT_HITPOINT_H
#define LSEVENT_HITPOINT_H

#include "../math/lsVector3.h"
#include "lsEvent.h"


class lsNode;

//! Ray Cast hit point event
/*!
 * This event is created when a Ray hits an object.
 *
 * */
class lsEvent_HitPoint : public lsEvent
{
public:
	lsEvent_HitPoint();
	virtual ~lsEvent_HitPoint();

	void Set_HitNode( lsNode* node );
	void Set_HitPoint( float x, float y, float z );
	void Set_HitNormal( float x, float y, float z );

private:
	lsNode* hit_node;
	lsVector3 hit_point;
	lsVector3 hit_normal;

};

#endif // LSEVENT_HITPOINT_H
