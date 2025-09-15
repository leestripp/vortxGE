// debug
#include "../vortx_debug.h"

// Physics
#include "lsBulletWorld.h"

// Scene
#include "../scene/lsScene.h"
#include "../scene/lsObject.h"

#include "lsFilterCallback.h"
#include "lsBulletObject.h"


lsBulletObject::lsBulletObject( lsObject* obj, btCollisionShape* bs, int maxProxies )
{
	object = obj;
	object->setBulletObject( this );

	mass = 0.0f;
	motion_type = LSBOT_STATIC;

	body = NULL;
	body_shape = bs;

	m_maxProxies = maxProxies;

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

		btTransform b_Transform;
		b_Transform.setIdentity();
		b_Transform.setOrigin( position );

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.0);

		btVector3 localInertia(0,0,0);
		if (isDynamic)
			body_shape->calculateLocalInertia( mass, localInertia );

		myMotionState = new btDefaultMotionState( b_Transform );
		btRigidBody::btRigidBodyConstructionInfo rbInfo( mass, myMotionState, body_shape, localInertia );

		body = new btRigidBody( rbInfo );
		if( body )
		{
			// set our user pointer to lsNode
			body->setUserPointer( object );

			//add the body to the dynamics world
			AddToSim();
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

	// Make sure we dont exced maxProxies - bullet will crash.
    if(( body )&&( dynamicsWorld ))
	{
		// dynamicsWorld
		if( dynamicsWorld->getCollisionWorld()->getNumCollisionObjects() < m_maxProxies )
		{
			dynamicsWorld->addRigidBody( body );
		} else
		{
			cout << "WARNING: lsBulletObject::AddToSim() - Exceded maxProxies" << endl;
		}
	}
}

void lsBulletObject::Apply_Physics( lsObject* obj )
{
	btScalar bm[16];
	lsMatrix4x4 scaleMatrix;
	lsMatrix4x4 bulletMatrix;
	
	btTransform trans;
	body->getMotionState()->getWorldTransform(trans);
	trans.getOpenGLMatrix( bm );
	
	// Create a matrix from the bullet transform
	bulletMatrix = bm;
	
	// Create a scale matrix from the object's scale property
	scaleMatrix.ScaleMatrix(obj->scale.x, obj->scale.y, obj->scale.z);
	
	// Combine them: T * R * S
	obj->ModelMatrix = bulletMatrix * scaleMatrix;
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

