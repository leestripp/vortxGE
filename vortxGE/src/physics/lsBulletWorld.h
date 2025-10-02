#ifndef LSBULLETWORLD_H_
#define LSBULLETWORLD_H_

#include <iostream>
#include <string>

// render
#include "../render/lsRender.h"

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
