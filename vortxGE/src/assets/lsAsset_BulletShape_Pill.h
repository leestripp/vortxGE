/**
 * @file lsAsset_BulletShape_Pill.h
 * @brief Capsule (pill) collision shape asset
 * 
 * Asset for creating a Bullet Physics capsule (pill) collision shape.
 * 
 * @author vortxGE
 * @date 2024
 * @see lsAsset
 * @see lsBulletShape
 */

#pragma once

#include "lsAsset.h"

/**
 * @class lsAsset_BulletShape_Pill
 * @brief Capsule/pill collision shape
 * 
 * Creates a capsule (cylinder with hemispherical ends) collision shape.
 * Ideal for character controllers as it slides smoothly over terrain
 * and handles slopes well.
 * 
 * Common uses:
 * - Player character controllers
 * - NPCs and creatures
 * - Vertical objects like poles
 * 
 * @note Asset type: LSAT_BULLETSHAPE
 * @note The height parameter is the cylinder portion only,
 *       total height = height + 2 * radius
 * 
 * @example
 * @code
 * // Create capsule with 0.5m radius and 1.8m cylinder height
 * lsAsset* shape = am->Add_Asset(LSAT_BULLETSHAPE_PILL);
 * lsAsset_BulletShape_Pill* pill = ...;
 * // Total height = 1.8 + 2*0.5 = 2.8m
 * @endcode
 */
class lsAsset_BulletShape_Pill : public lsAsset
{
public:
	/**
	 * @brief Construct capsule shape
	 * 
	 * @param radius Radius of the capsule
	 * @param height Height of the cylindrical portion
	 */
    lsAsset_BulletShape_Pill(float radius, float height);
    
    /// Destructor
    virtual ~lsAsset_BulletShape_Pill();

    //=========================================================================
    // lsAsset Override
    //=========================================================================
    
    /**
     * @brief Create Bullet capsule shape
     */
    void AllocAsset() override;
    
    /**
     * @brief Destroy shape
     */
    void FreeAsset() override;

private:
    float m_radius;	///< Capsule radius
    float m_height;	///< Cylinder height (excludes hemispheres)
};
