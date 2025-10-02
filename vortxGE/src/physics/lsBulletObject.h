#pragma once

#include <iostream>
#include <string>

// render
#include "../render/lsRender.h"

// bullet
#include <bullet/btBulletDynamicsCommon.h>


// How objects move
const int LSBOT_STATIC		=	0;
const int LSBOT_DYNAMIC     =	1;
const int LSBOT_KINEMATIC	=	2;

class lsObject;

class lsBulletObject
{
public:
	lsBulletObject( lsObject* obj, btCollisionShape* bs, int maxProxies );
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

    btRigidBody* getBody() { return body; }

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
	int			m_maxProxies;
};
