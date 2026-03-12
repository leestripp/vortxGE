/**
 * @file lsAsset_BulletShape_Plane.h
 * @brief Infinite plane collision shape asset
 * 
 * Asset for creating an infinite plane collision shape for physics.
 * 
 * @author vortxGE
 * @date 2024
 * @see lsAsset
 * @see lsBulletShape
 */

#pragma once

// math
#include "../math/lsVector3.h"

#include "lsAsset.h"

/**
 * @class lsAsset_BulletShape_Plane
 * @brief Infinite plane collision shape
 * 
 * Creates an infinite plane collision shape. The plane extends
 * infinitely in all directions and is useful for:
 * - Ground/floor surfaces
 * - Walls (with appropriate orientation)
 * - Ceilings
 * 
 * @note Asset type: LSAT_BULLETSHAPE
 * @note The plane is infinite - objects can slide indefinitely
 * @note For bounded floors, use a large box instead
 * 
 * @example
 * @code
 * // Create horizontal floor (pointing up)
 * lsAsset* floor = am->Add_Asset(LSAT_BULLETSHAPE_PLANE);
 * // Points in Y+ direction (0, 1, 0)
 * @endcode
 */
class lsAsset_BulletShape_Plane : public lsAsset
{
public:
	/**
	 * @brief Construct infinite plane
	 * 
	 * @param vectorX Normal X component
	 * @param vectorY Normal Y component
	 * @param vectorZ Normal Z component
	 */
    lsAsset_BulletShape_Plane( float vectorX, float vectorY, float vectorZ );
    
    /// Destructor
    virtual ~lsAsset_BulletShape_Plane();

    //=========================================================================
    // lsAsset Override
    //=========================================================================
    
    /**
     * @brief Create Bullet plane shape
     */
    void AllocAsset();
    
    /**
     * @brief Destroy shape
     */
    void FreeAsset();

private:
    /// Plane normal vector (direction)
    lsVector3 normal;

};
