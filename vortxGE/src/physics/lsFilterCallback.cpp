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

#include "../vortx_debug.h"

// Scene
#include "../scene/lsObject.h"

#include "lsBulletWorld.h"
#include "lsFilterCallback.h"


lsFilterCallback::lsFilterCallback()
{

}

lsFilterCallback::~lsFilterCallback()
{
}


bool lsFilterCallback::needBroadphaseCollision( btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1 ) const
{
	bool collides = (proxy0->m_collisionFilterGroup & proxy1->m_collisionFilterMask) != 0;
	collides = collides && (proxy1->m_collisionFilterGroup & proxy0->m_collisionFilterMask);

	//add some additional logic here that modifies 'collides'
	btCollisionObject* obA = static_cast<btCollisionObject*>(proxy0->m_clientObject);
	btCollisionObject* obB = static_cast<btCollisionObject*>(proxy1->m_clientObject);

	// Get Scene nodes
	lsObject *obj_A = (lsObject*)obA->getUserPointer();
	lsObject *obj_B = (lsObject*)obB->getUserPointer();
	if(( obj_A )&&( obj_B ))
	{
		switch( obj_A->getNode_type() )
		{
			case LSNT_PARTICLE:
				// check physics disabled
				// if( obj_A->getHide() ) collides = false;
				break;
		}
		switch( obj_B->getNode_type() )
		{
			case LSNT_PARTICLE:
				// check physics disabled
				// if( obj_B->getHide() ) collides = false;
				break;
		}

#ifdef VORTX_DEBUG_PHYSICS
		/* Debug */
		cout << "Object A : ";
		cout << obj_A->getName();
		cout << "  -  hit Object B : ";
		cout << obj_B->getName();
		cout << endl;
#endif
	}
	return collides;
}

