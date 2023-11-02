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

#ifndef LSBULLETWORLD_H_
#define LSBULLETWORLD_H_

#include <iostream>
#include <string>

// opengl
#include "../opengl/lsOpenGL.h"

// bullet
#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h>
#include <bullet/BulletDynamics/Character/btKinematicCharacterController.h>
#include <bullet/BulletCollision/CollisionDispatch/btGhostObject.h>

class lsScene;
class lsTimer;


using namespace std;

//! Bullet Physics
/*!
 *
 * */
class lsBulletWorld
{
public:
	lsBulletWorld( lsScene* scn );
	virtual ~lsBulletWorld();

	void Init();
	void CleanUp();
	void BulletStep( int steps, double delta );

	// Tick callback
	static void lsTickCallback( btDynamicsWorld *world, btScalar timeStep );
	void lsProcessCallback( btScalar timeStep );


	btDiscreteDynamicsWorld *getDynamicsWorld() const
	{
		return dynamicsWorld;
	}

	void setDynamicsWorld(btDiscreteDynamicsWorld *dynamicsWorld)
	{
		this->dynamicsWorld = dynamicsWorld;
	}

	void setWorld_size( float arg1 )
	{
		world_size = arg1;
	}

	void setMax_proxies( int arg1 )
	{
		maxProxies = arg1;
	}

	int getMax_proxies() const
	{
		return maxProxies;
	}


private:
	lsScene *scene;

	btDiscreteDynamicsWorld *dynamicsWorld;
	btDefaultCollisionConfiguration *collisionConfiguration;
	btCollisionDispatcher *dispatcher;
	btOverlappingPairCache *overlappingPairCache;
	btOverlapFilterCallback *filterCallback;
	btAxisSweep3 *axis_sweep3;
	btSequentialImpulseConstraintSolver *solver;
	btGhostPairCallback *ghostpaircallback;

	float world_size;
	int maxProxies;
	btScalar time_step;

};

#endif /* LSBULLETWORLD_H_ */
