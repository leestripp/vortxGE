/**
 * @file lsAsset_BulletShape_ConvexHull.h
 * @brief Convex hull collision shape asset
 * 
 * Asset for creating a convex hull collision shape from mesh data.
 * 
 * @author vortxGE
 * @date 2024
 * @see lsAsset
 * @see lsAsset_Mesh
 * @see lsBulletShape
 */

#pragma once

#include "lsAsset.h"

class lsAsset_Mesh;

/**
 * @class lsAsset_BulletShape_ConvexHull
 * @brief Convex hull collision shape
 * 
 * Creates a convex hull shape from mesh geometry. A convex hull is
 * the smallest convex shape that contains all vertices of the mesh.
 * 
 * This is more accurate than a box or capsule for complex shapes,
 * but still relatively fast for collision detection.
 * 
 * Use for: Rocks, debris, complex furniture, irregular objects
 * 
 * @note Asset type: LSAT_BULLETSHAPE
 * @note For concave shapes, use TriMesh (static only) or compound shapes
 * 
 * @example
 * @code
 * lsAsset* meshAsset = am->Add_Asset(LSAT_MESH, "meshes/rock.glb");
 * lsAsset* shape = am->Add_Asset(LSAT_BULLETSHAPE_CONVEXHULL);
 * // Constructor takes mesh asset
 * @endcode
 */
class lsAsset_BulletShape_ConvexHull : public lsAsset
{
public:
	/**
	 * @brief Construct convex hull from mesh
	 * @param meshAsset Mesh to create hull from
	 */
    lsAsset_BulletShape_ConvexHull(lsAsset_Mesh* meshAsset);
    
    /// Destructor
    virtual ~lsAsset_BulletShape_ConvexHull();

    //=========================================================================
    // lsAsset Override
    //=========================================================================
    
    /**
     * @brief Create convex hull from mesh
     */
    void AllocAsset() override;
    
    /**
     * @brief Destroy shape
     */
    void FreeAsset() override;

private:
    /// Source mesh for hull generation
    lsAsset_Mesh* m_meshAsset;
};
