/**
 * @file lsRayCast.h
 * @brief Physics ray casting for scene queries
 * 
 * lsRayCast performs ray intersection tests against the
 * physics world. Used for:
 * - Ray picking (selecting objects with mouse)
 * - Line of sight checks
 * - Ground detection
 * - Distance queries
 * 
 * @section usage Usage
 * @code
 * lsRayCast* ray = new lsRayCast(physics->getDynamicsWorld());
 * 
 * // Cast ray from camera through mouse position
 * ray->castRay(from, to);
 * 
 * // Check hit_pos and hit_norm for results
 * @endcode
 * 
 * @see lsBulletWorld for physics world access
 */

#ifndef LSRAYCAST_H
#define LSRAYCAST_H

#include <iostream>
#include <string>

#include <bullet/btBulletDynamicsCommon.h>

/**
 * @class lsRayCast
 * @brief Ray casting for physics collision detection
 * 
 * Casts rays into the physics world to detect intersections.
 * Useful for picking, line-of-sight, and ground detection.
 * 
 * @section results Results
 * 
 * After castRay():
 * - hit_pos: World position of intersection
 * - hit_norm: Surface normal at intersection point
 */
class lsRayCast
{
    public:
        /**
         * @brief Construct ray caster
         * @param dw Pointer to dynamics world
         */
        lsRayCast( btDiscreteDynamicsWorld* dw );
        
        /**
         * @brief Destructor
         */
        virtual ~lsRayCast();

        /**
         * @brief Cast a ray
         * 
         * Performs ray intersection test against all collision
         * objects in the physics world.
         * 
         * @param from Start point (world coordinates)
         * @param to End point (world coordinates)
         * 
         * @note Results stored in hit_pos and hit_norm
         */
        void castRay( const btVector3& from,const btVector3& to );

    private:
        /** @brief Dynamics world reference */
        btDiscreteDynamicsWorld* world;

        /** @brief Hit position in world space */
        btVector3 hit_pos;
        
        /** @brief Surface normal at hit point */
        btVector3 hit_norm;

};

#endif // LSRAYCAST_H
