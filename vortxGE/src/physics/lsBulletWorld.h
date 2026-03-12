/**
 * @file lsBulletWorld.h
 * @brief Bullet Physics world management
 * 
 * lsBulletWorld manages the Bullet Physics simulation world.
 * It provides:
 * - Physics world initialization
 * - Collision detection setup
 * - Simulation stepping
 * - Ghost object pairs (for triggers/overlap detection)
 * 
 * @section usage Usage
 * @code
 * // Create physics world
 * lsBulletWorld* physics = new lsBulletWorld(scene);
 * physics->setWorld_size(1000.0f);
 * physics->setMax_proxies(1024);
 * physics->Init();
 * 
 * // In update loop:
 * physics->BulletStep(1, deltaTime);
 * @endcode
 * 
 * @note Physics world is automatically created when lsScene is constructed
 * @see lsBulletObject for physics-enabled objects
 * @see lsRayCast for ray casting
 */

#ifndef LSBULLETWORLD_H_
#define LSBULLETWORLD_H_

#include <iostream>
#include <string>

// render
#include "../render/lsRender.h"

// Bullet Physics headers
#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h>
#include <bullet/BulletDynamics/Character/btKinematicCharacterController.h>
#include <bullet/BulletCollision/CollisionDispatch/btGhostObject.h>

// Forward declarations
class lsScene;
class lsTimer;

/**
 * @class lsBulletWorld
 * @brief Bullet Physics world container
 * 
 * Manages the Bullet Physics simulation including:
 * - Collision detection
 * - Rigid body dynamics
 * - Character controllers
 * - Trigger volumes (ghost objects)
 * 
 * @section components Components
 * 
 * - btDiscreteDynamicsWorld: Main simulation world
 * - btDefaultCollisionConfiguration: Collision setup
 * - btCollisionDispatcher: Collision dispatch
 * - btOverlappingPairCache: Broadphase collision
 * - btSequentialImpulseConstraintSolver: Solver
 */
class lsBulletWorld
{
public:
    /**
     * @brief Construct physics world
     * @param scn Pointer to scene
     */
	lsBulletWorld( lsScene* scn );
	
    /**
     * @brief Destructor
     * 
     * Cleans up all Bullet resources.
     */
	virtual ~lsBulletWorld();
	
    /**
     * @brief Initialize physics world
     * 
     * Creates all Bullet components:
     * - Collision configuration
     * - Dispatcher
     * - Broadphase
     * - Solver
     * - Dynamics world
     */
	void Init();
	
    /**
     * @brief Cleanup physics world
     * 
     * Releases all Bullet resources.
     */
	void CleanUp();
	
    /**
     * @brief Step the simulation
     * 
     * Advances physics simulation by delta time.
     * 
     * @param steps Number of sub-steps
     * @param delta Time step in seconds
     */
	void BulletStep( int steps, double delta );
	
    //=========================================================================
	// Callbacks
	//=========================================================================
	
    /**
     * @brief Static tick callback
     * 
     * Called by Bullet each simulation step.
     * 
     * @param world Dynamics world
     * @param timeStep Current time step
     */
	static void lsTickCallback( btDynamicsWorld *world, btScalar timeStep );
	
    /**
     * @brief Process tick callback
     * 
     * Handles per-frame physics processing.
     * 
     * @param timeStep Current time step
     */
	void lsProcessCallback( btScalar timeStep );
	
    //=========================================================================
	// Accessors
	//=========================================================================
	
    /**
     * @brief Get dynamics world
     * @return Pointer to btDiscreteDynamicsWorld
     */
	btDiscreteDynamicsWorld *getDynamicsWorld() const
	{
		return dynamicsWorld;
	}
	
    /**
     * @brief Set dynamics world
     * @param dynamicsWorld Pointer to world
     */
	void setDynamicsWorld(btDiscreteDynamicsWorld *dynamicsWorld)
	{
		this->dynamicsWorld = dynamicsWorld;
	}
	
    /**
     * @brief Set world size
     * 
     * Size of the physics simulation bounding box.
     * 
     * @param arg1 World size (extends -arg1 to +arg1)
     */
	void setWorld_size( float arg1 )
	{
		world_size = arg1;
	}
	
    /**
     * @brief Set max proxies
     * 
     * Maximum number of collision proxies (broadphase).
     * 
     * @param arg1 Max proxy count
     */
	void setMax_proxies( int arg1 )
	{
		maxProxies = arg1;
	}
	
    /**
     * @brief Get max proxies
     * @return Maximum proxy count
     */
	int getMax_proxies() const
	{
		return maxProxies;
	}
	
private:
    //=========================================================================
	// Private Members
	//=========================================================================
	
    /** @brief Parent scene */
	lsScene *scene;
	
    //=========================================================================
	// Bullet Components
	//=========================================================================
	
    /** @brief Main dynamics world */
	btDiscreteDynamicsWorld *dynamicsWorld;
    
    /** @brief Collision configuration */
	btDefaultCollisionConfiguration *collisionConfiguration;
    
    /** @brief Collision dispatcher */
	btCollisionDispatcher *dispatcher;
    
    /** @brief Broadphase collision detection */
	btOverlappingPairCache *overlappingPairCache;
    
    /** @brief Collision filter callback */
	btOverlapFilterCallback *filterCallback;
    
    /** @brief Axis sweep broadphase */
	btAxisSweep3 *axis_sweep3;
    
    /** @brief Constraint solver */
	btSequentialImpulseConstraintSolver *solver;
    
    /** @brief Ghost pair callback for triggers */
	btGhostPairCallback *ghostpaircallback;
	
    //=========================================================================
	// Settings
	//=========================================================================
	
    /** @brief World size */
	float world_size;
    
    /** @brief Max collision proxies */
	int maxProxies;
    
    /** @brief Time step accumulator */
	btScalar time_step;
};

#endif /* LSBULLETWORLD_H_ */
