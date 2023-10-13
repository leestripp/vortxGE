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

#ifndef LSBULLETOBJECT_H_
#define LSBULLETOBJECT_H_

#include <iostream>
#include <string>

// opengl
#include "../opengl/lsOpenGL.h"

// bullet
#include <bullet/btBulletDynamicsCommon.h>


// How objects move
const int LSBOT_STATIC		=	0;
const int LSBOT_DYNAMIC     =	1;
const int LSBOT_KINEMATIC	=	2;

class lsObject;

//! Bullet Physics
/*!
 * Creates a bullet shape to attach to a lsNode.
 *
 * */
class lsBulletObject
{
public:
	lsBulletObject( lsObject* obj, btCollisionShape* bs );
	virtual ~lsBulletObject();

	void Init( float basemass );
	void Apply_Physics( lsObject* obj );
	void Reset_Physics( lsObject* obj );

    void RemoveFromSim();
    void AddToSim();

	// Movement
	void Set_Position(float x, float y, float z);
	void Set_Rotation(float x, float y, float z);
	void Set_Gravity(float x, float y, float z);
	void Set_Velocity(float x, float y, float z);

private:
	lsObject *object;

	btCollisionShape *body_shape;
	btRigidBody *body;
	btDefaultMotionState *myMotionState;
	// From lsBulletWorld
	btDiscreteDynamicsWorld *dynamicsWorld;

public:
	int         motion_type;
	btScalar 	mass;
	btVector3 	position;

};

#endif /* LSBULLETOBJECT_H_ */
