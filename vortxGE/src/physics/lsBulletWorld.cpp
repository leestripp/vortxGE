/**
 * @file lsBulletWorld.cpp
 * @brief Bullet physics world implementation
 * 
 * This file implements the physics world that manages all physics
 * simulation using the Bullet Physics engine.
 * 
 * Key components:
 * - Collision detection dispatcher
 * - Broadphase collision detection (AxisSweep3)
 * - Constraint solver for joints
 * - Discrete dynamics world
 * - Collision filtering
 * 
 * @see lsBulletWorld.h
 * @see lsBulletObject.h
 * @see lsFilterCallback.h
 */

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


/**
 * @brief Construct physics world
 * 
 * Initializes Bullet physics world with default parameters:
 * - Max collision proxies: 1024
 * - World bounds: 10000 units in each axis
 * 
 * @param scn Pointer to scene (for object synchronization)
 */
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

/**
 * @brief Initialize Bullet physics world
 * 
 * Sets up the Bullet Physics simulation environment:
 * 
 * 1. Collision Configuration:
 *    - btDefaultCollisionConfiguration: Standard memory/setup
 * 
 * 2. Dispatcher:
 *    - btCollisionDispatcher: Manages collision detection
 * 
 * 3. Broadphase:
 *    - btAxisSweep3: Efficient AABB sweep and prune
 *    - Manages collision proxy overlap detection
 *    - Ghost pair callback for character controllers
 * 
 * 4. Solver:
 *    - btSequentialImpulseConstraintSolver: Resolves contacts
 * 
 * 5. Dynamics World:
 *    - btDiscreteDynamicsWorld: Main simulation world
 *    - Gravity: -19.6 (2x Earth gravity for game feel)
 *    - Internal tick callback for synchronization
 * 
 * 6. Collision Filtering:
 *    - Custom lsFilterCallback for group/mask filtering
 * 
 * @note Must be called before any physics simulation
 */

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
	
	// TODO: Change to threaded.
	//the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	solver = new btSequentialImpulseConstraintSolver;
	
	dynamicsWorld = new btDiscreteDynamicsWorld( dispatcher, axis_sweep3, solver, collisionConfiguration );
	
	// Realistic gravity is 9.8 m/s squared.
	dynamicsWorld->setGravity(btVector3(0, -9.8*2.0, 0));
	
	// set callbacks
	dynamicsWorld->setInternalTickCallback( &lsTickCallback, static_cast<void *>(this) );
	
	filterCallback = new lsFilterCallback();
	dynamicsWorld->getPairCache()->setOverlapFilterCallback( filterCallback );
}


/**
 * @brief Step the physics simulation
 * 
 * Advances the physics simulation by the specified time delta.
 * 
 * Simulation parameters:
 * - time_step: Delta time in seconds (converted from milliseconds)
 * - steps: Maximum sub-steps to catch up
 * - fixedTimeStep: 1/60 second (60 Hz physics)
 * 
 * Bullet's stepSimulation handles:
 * 1. Predictive motion integration
 * 2. Broadphase collision detection
 * 3. Narrowphase collision detection
 * 4. Contact point generation
 * 5. Constraint solving
 * 6. Position/velocity updates
 * 
 * @param steps Maximum number of sub-steps
 * @param delta Time since last frame (milliseconds)
 * 
 * @note Fixed timestep ensures deterministic physics
 */

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

