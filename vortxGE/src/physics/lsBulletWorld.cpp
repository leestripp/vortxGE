// timer
#include "../timers/lsTimer.h"

// scene
#include "../scene/lsScene.h"
#include "../scene/lsNode.h"

// events
#include "../events/lsEventManager.h"
#include "../events/lsEvent.h"
#include "../events/lsEvent_HitPoint.h"

#include "lsFilterCallback.h"
#include "lsBulletWorld.h"


lsBulletWorld::lsBulletWorld( lsScene* scn )
{
	scene = scn;

	maxProxies = 1024;
	world_size = 10000;

    dynamicsWorld = NULL;
    solver = NULL;
    overlappingPairCache = NULL;
    dispatcher = NULL;
    collisionConfiguration = NULL;

}

lsBulletWorld::~lsBulletWorld()
{
	if( dynamicsWorld ) delete dynamicsWorld;
	if( solver ) delete solver;
	if( axis_sweep3 ) delete axis_sweep3;
	if( ghostpaircallback ) delete ghostpaircallback;
    if( dispatcher ) delete dispatcher;
	if( collisionConfiguration ) delete collisionConfiguration;
	if( filterCallback ) delete filterCallback;
}

void lsBulletWorld::Init()
{
	//collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
	collisionConfiguration = new btDefaultCollisionConfiguration();

	//use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	dispatcher = new btCollisionDispatcher( collisionConfiguration );

	btVector3 worldAabbMin( -world_size,-world_size,-world_size );
	btVector3 worldAabbMax( world_size,world_size,world_size );

	axis_sweep3 = new btAxisSweep3( worldAabbMin, worldAabbMax, maxProxies );
	overlappingPairCache = axis_sweep3->getOverlappingPairCache();
    ghostpaircallback = new btGhostPairCallback();
	axis_sweep3->getOverlappingPairCache()->setInternalGhostPairCallback( ghostpaircallback );

	//the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	solver = new btSequentialImpulseConstraintSolver;

	dynamicsWorld = new btDiscreteDynamicsWorld( dispatcher, axis_sweep3, solver, collisionConfiguration );
	dynamicsWorld->setGravity(btVector3(0,-10,0));

	// set callbacks
	dynamicsWorld->setInternalTickCallback( &lsTickCallback, static_cast<void *>(this) );

	filterCallback = new lsFilterCallback();
	dynamicsWorld->getPairCache()->setOverlapFilterCallback( filterCallback );

}


void lsBulletWorld::BulletStep( int steps, double delta )
{
    time_step = delta / 1000.0;
	// btDynamicsWorld::stepSimulation( btScalar timeStep, int maxSubSteps=1, btScalar fixedTimeStep=btScalar(1.)/btScalar(60.) );
    dynamicsWorld->stepSimulation( time_step, steps, 1.0 / 60.0 );
}


void lsBulletWorld::lsTickCallback( btDynamicsWorld *world, btScalar timeStep )
{
	lsBulletWorld *w = static_cast<lsBulletWorld *>(world->getWorldUserInfo());
	w->lsProcessCallback( timeStep );

}

void lsBulletWorld::lsProcessCallback( __attribute__((unused)) btScalar timeStep )
{
	// local instance callback
	// cout << scene->getName() << " : world just ticked by " << timeStep << " seconds" << endl;

	lsNode* obj_A;
	lsNode* obj_B;

	// get hits
	int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
	for( int i=0; i<numManifolds; i++ )
	{
		btPersistentManifold* contactManifold =  dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
		const btCollisionObject* obA = static_cast<const btCollisionObject*>(contactManifold->getBody0());
		const btCollisionObject* obB = static_cast<const btCollisionObject*>(contactManifold->getBody1());

		int numContacts = contactManifold->getNumContacts();
		for( int j=0; j<numContacts; j++ )
		{
			btManifoldPoint& pt = contactManifold->getContactPoint(j);
			if( pt.getDistance() < 0.0 )
			{
				// const btVector3& ptA = pt.getPositionWorldOnA();
				const btVector3& ptB = pt.getPositionWorldOnB();
				const btVector3& normalOnB = pt.m_normalWorldOnB;

				// Get Scene nodes
				obj_A = (lsNode*)obA->getUserPointer();
				obj_B = (lsNode*)obB->getUserPointer();
				if(( obj_A )&&( obj_B ))
				{
					if( obj_B->Get_EventManager() )
					{
						// Add event
						lsEvent_HitPoint* e = new lsEvent_HitPoint();
						if( e )
						{
							e->Set_HitNode( obj_A );
							e->Set_HitPoint( ptB.getX(), ptB.getY(), ptB.getZ() );
							e->Set_HitNormal( normalOnB.getX(), normalOnB.getY(), normalOnB.getZ() );

							obj_B->Get_EventManager()->Add_Event( e );
						}
					}
				}
			}
		}
	}

}

