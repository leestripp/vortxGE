/**
 * @file lsBulletObject.h
 * @brief Physics body for rigid body simulation
 * 
 * lsBulletObject wraps a Bullet Physics rigid body and syncs
 * it with a scene object. It provides:
 * - Rigid body creation from collision shapes
 * - Position/rotation synchronization
 * - Velocity and gravity control
 * - Kinematic and dynamic motion types
 * 
 * @section motion_types Motion Types
 * 
 * - LSBOT_STATIC: Immovable (infinite mass)
 * - LSBOT_DYNAMIC: Physics-controlled movement
 * - LSBOT_KINEMATIC: Programmatic movement (not physics-driven)
 * 
 * @section usage Usage
 * @code
 * // Create physics body for object
 * lsBulletObject* body = new lsBulletObject(obj, nullptr, 1024);
 * 
 * // Create box shape
 * btBoxShape* shape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));
 * body->Init(1.0f);  // Mass of 1kg
 * 
 * // Enable kinematic movement
 * body->motion_type = LSBOT_KINEMATIC;
 * @endcode
 * 
 * @see lsBulletWorld for physics world management
 * @see lsAsset_BulletShape_* for collision shapes
 */

#pragma once

#include <iostream>
#include <string>

// render
#include "../render/lsRender.h"

// Bullet
#include <bullet/btBulletDynamicsCommon.h>

//=========================================================================
// Motion Type Constants
//=========================================================================

/** @brief Static object (infinite mass, immovable) */
const int LSBOT_STATIC		=	0;
/** @brief Dynamic object (physics-controlled) */
const int LSBOT_DYNAMIC     =	1;
/** @brief Kinematic object (programmatic movement) */
const int LSBOT_KINEMATIC	=	2;

// Forward declaration
class lsObject;

/**
 * @class lsBulletObject
 * @brief Rigid body wrapper for Bullet Physics
 * 
 * Connects a scene object to its physics simulation.
 * Handles:
 * - Body creation and initialization
 * - Sync between physics and visual transform
 * - Force/velocity application
 * 
 * @section synchronization Synchronization
 * 
 * Each frame:
 * 1. Apply_Physics() copies position/rotation from physics to object
 * 2. Object's visual transform matches physics body
 */
class lsBulletObject
{
public:
    /**
     * @brief Construct physics object
     * 
     * @param obj Parent scene object
     * @param bs Collision shape (can be nullptr, set later)
     * @param maxProxies Maximum collision proxies
     */
	lsBulletObject( lsObject* obj, btCollisionShape* bs, int maxProxies );
	
    /**
     * @brief Destructor
     * 
     * Removes body from simulation and frees resources.
     */
	virtual ~lsBulletObject();
	
    /**
     * @brief Initialize rigid body
     * 
     * Creates the Bullet rigid body with the collision shape.
     * 
     * @param basemass Mass in kilograms (0 for static)
     */
	void Init( float basemass );
	
    /**
     * @brief Apply physics to object
     * 
     * Copies transform from physics body to scene object.
     * Called each frame to sync visual with physics.
     * 
     * @param obj Scene object to update
     */
	void Apply_Physics( lsObject* obj );
	
    /**
     * @brief Reset physics body
     * 
     * Resets position, rotation, and velocity.
     * 
     * @param obj Scene object to reset from
     */
	void Reset_Physics( lsObject* obj );

    /**
     * @brief Remove from simulation
     * 
     * Temporarily disables physics for this body.
     */
    void RemoveFromSim();
    
    /**
     * @brief Add to simulation
     * 
     * Re-enables physics for this body.
     */
    void AddToSim();

	//=========================================================================
	// Transform
	//=========================================================================
	
    /**
     * @brief Set physics position
     * 
     * @param x X coordinate
     * @param y Y coordinate
     * @param z Z coordinate
     */
	void Set_Position(float x, float y, float z);
	
    /**
     * @brief Set physics rotation
     * 
     * @param x Pitch (degrees)
     * @param y Yaw (degrees)
     * @param z Roll (degrees)
     */
	void Set_Rotation(float x, float y, float z);
	
    /**
     * @brief Set gravity
     * 
     * @param x X component
     * @param y Y component  
     * @param z Z component
     */
	void Set_Gravity(float x, float y, float z);
	
    /**
     * @brief Set linear velocity
     * 
     * @param x X component
     * @param y Y component
     * @param z Z component
     */
	void Set_Velocity(float x, float y, float z);

    //=========================================================================
	// Accessors
	//=========================================================================
	
    /**
     * @brief Get Bullet rigid body
     * @return Pointer to btRigidBody
     */
    btRigidBody* getBody() { return body; }

private:
    //=========================================================================
	// Private Members
	//=========================================================================
	
    /** @brief Parent scene object */
	lsObject *object;

	/** @brief Collision shape */
	btCollisionShape *body_shape;
    
    /** @brief Bullet rigid body */
	btRigidBody *body;
    
    /** @brief Motion state for transform sync */
	btDefaultMotionState *myMotionState;
    
    /** @brief Dynamics world reference */
	btDiscreteDynamicsWorld *dynamicsWorld;

public:
    //=========================================================================
    // Public Data Members
    //=========================================================================
	
    /** @brief Motion type (LSBOT_STATIC/DYNAMIC/KINEMATIC) */
	int         motion_type;
    
    /** @brief Object mass (0 = infinite/static) */
	btScalar 	mass;
    
    /** @brief Physics position */
	btVector3 	position;
    
    /** @brief Maximum collision proxies */
	int			m_maxProxies;
};
