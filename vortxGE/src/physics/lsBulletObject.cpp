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

// debug
#include "../vortx_debug.h"

// Physics
#include "lsBulletWorld.h"

// Scene
#include "../scene/lsScene.h"
#include "../scene/lsObject.h"

#include "lsFilterCallback.h"
#include "lsBulletObject.h"


lsBulletObject::lsBulletObject( lsObject* obj, btCollisionShape* bs )
{
	object = obj;
	object->setBulletObject( this );

	mass = 0.0f;
	motion_type = LSBOT_STATIC;

	body = NULL;
	body_shape = bs;

    // get our world
    lsScene *scn;
    lsBulletWorld *bw = NULL;
    dynamicsWorld = NULL;

    scn = obj->Get_Scene();
    if( scn )
    {
        bw = scn->getBulletworld();
        if( bw )
        {
            dynamicsWorld = bw->getDynamicsWorld();
        }
    }
}

lsBulletObject::~lsBulletObject()
{
	if( body )
	{
		if( dynamicsWorld ) dynamicsWorld->removeRigidBody( body );
		delete body->getMotionState();
		delete body;
	}
}


void lsBulletObject::Init( float basemass )
{
#ifdef VORTX_DEBUG_PHYSICS
    cout << "lsBulletObject::Init Mass=" << basemass << endl;
#endif

    mass = basemass;

	if(( body_shape )&&( object ))
	{
		position = btVector3( object->position.x, object->position.y, object->position.z );

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin( position );

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.0);

		btVector3 localInertia(0,0,0);
		if (isDynamic)
			body_shape->calculateLocalInertia( mass, localInertia );

		myMotionState = new btDefaultMotionState( groundTransform );
		btRigidBody::btRigidBodyConstructionInfo rbInfo( mass, myMotionState, body_shape, localInertia );

		body = new btRigidBody( rbInfo );
		if( body )
		{
			// set our user pointer to lsNode
			body->setUserPointer( object );

			//add the body to the dynamics world
			if( dynamicsWorld ) dynamicsWorld->addRigidBody( body );
		}
	}
}

/*! RemoveFromSim
 Remove the bullet object body from the simulation, but do not delete it.
 */
void lsBulletObject::RemoveFromSim()
{
#ifdef VORTX_DEBUG_PHYSICS
    cout << "lsBulletObject::RemoveFromSim" << endl;
#endif

    if( body )
	{
		if( dynamicsWorld ) dynamicsWorld->removeRigidBody( body );
	}
}

void lsBulletObject::AddToSim()
{
#ifdef VORTX_DEBUG_PHYSICS
    cout << "lsBulletObject::AddToSim" << endl;
#endif

    if( body )
	{
		if( dynamicsWorld ) dynamicsWorld->addRigidBody( body );
	}
}

void lsBulletObject::Apply_Physics( lsObject* obj )
{
	btScalar bm[16];

	btTransform trans;
	body->getMotionState()->getWorldTransform(trans);
	trans.getOpenGLMatrix( bm );

	// New Matrix
	obj->ModelMatrix.LoadIdentity();
	obj->ModelMatrix *= bm;

}

/*! Reset physics
 Reset or move physics to the Objects local position
 Reset rotation and velocity.
 */
void lsBulletObject::Reset_Physics( lsObject* obj )
{
	Set_Position( obj->position.x, obj->position.y, obj->position.z );
    Set_Rotation( obj->rotation.x, obj->rotation.y, obj->rotation.z );
    Set_Velocity( 0, 0, 0 );
}

void lsBulletObject::Set_Position(float x, float y, float z)
{
	btTransform trans;
	trans.setIdentity();
	trans.setOrigin( btVector3(x,y,z) );

	myMotionState->setWorldTransform( trans );
	body->setCenterOfMassTransform( trans );
	body->clearForces();
	body->activate();
}

void lsBulletObject::Set_Rotation(float x, float y, float z)
{
    btTransform trans;
	btQuaternion qrot;

	qrot.setEulerZYX( z, y, x );
    myMotionState->getWorldTransform( trans );
	trans.setRotation( qrot );

	myMotionState->setWorldTransform( trans );
	body->setCenterOfMassTransform( trans );
	body->clearForces();
	body->activate();
}

void lsBulletObject::Set_Gravity(float x, float y, float z)
{
	if( body )
	{
		body->setGravity( btVector3(x,y,z) );
	}
}

void lsBulletObject::Set_Velocity(float x, float y, float z)
{
	if( body )
	{
		body->setLinearVelocity( btVector3(x,y,z) );
	}
}

