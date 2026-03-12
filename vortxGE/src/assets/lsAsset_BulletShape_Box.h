/**
 * @file lsAsset_BulletShape_Box.h
 * @brief Box collision shape asset
 * 
 * Asset for creating a Bullet Physics box (AABB) collision shape.
 * 
 * @author vortxGE
 * @date 2024
 * @see lsAsset
 * @see lsBulletShape
 */

#ifndef vortxGE_lsAsset_BulletShape_Box_h
#define vortxGE_lsAsset_BulletShape_Box_h

// math
#include "../math/lsVector3.h"

#include "lsAsset.h"

/**
 * @class lsAsset_BulletShape_Box
 * @brief Box collision shape
 * 
 * Creates an axis-aligned box collision shape for physics simulation.
 * A box is the simplest and fastest collision shape, ideal for
 * rectangular objects like walls, floors, and crates.
 * 
 * The box extends half the specified size in each direction from
 * its center (matching Bullet Physics convention).
 * 
 * @note Asset type: LSAT_BULLETSHAPE
 * @note For dynamic objects, consider using a smaller convex hull
 * 
 * @example
 * @code
 * // Create 2x4x2 meter box shape
 * lsAsset* shape = am->Add_Asset(LSAT_BULLETSHAPE_BOX);
 * lsAsset_BulletShape_Box* box = dynamic_cast<lsAsset_BulletShape_Box*>(shape);
 * // Note: constructor takes half-extents (1, 2, 1)
 * @endcode
 */
class lsAsset_BulletShape_Box : public lsAsset
{
public:
	/**
	 * @brief Construct box shape
	 * 
	 * @param x Half-extent in X direction
	 * @param y Half-extent in Y direction
	 * @param z Half-extent in Z direction
	 */
    lsAsset_BulletShape_Box( float x, float y, float z );
    
    /// Destructor
    virtual ~lsAsset_BulletShape_Box();

    //=========================================================================
    // lsAsset Override
    //=========================================================================
    
    /**
     * @brief Create Bullet box shape
     */
    void AllocAsset();
    
    /**
     * @brief Destroy Bullet shape
     */
    void FreeAsset();

private:
    /// Half-extents of the box
    lsVector3 box;

};


#endif
