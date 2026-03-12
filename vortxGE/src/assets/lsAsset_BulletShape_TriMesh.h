/**
 * @file lsAsset_BulletShape_TriMesh.h
 * @brief Triangle mesh collision shape asset
 * 
 * Asset for creating a static triangle mesh collision shape from mesh data.
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
 * @class lsAsset_BulletShape_TriMesh
 * @brief Triangle mesh collision shape
 * 
 * Creates a static triangle mesh shape from mesh geometry. This allows
 * exact collision with complex static geometry like:
 * - Terrain
 * - Architectural meshes
 * - Detailed static models
 * 
 * @warning This shape can ONLY be used for static objects (mass = 0).
 * For dynamic objects, use ConvexHull instead.
 * 
 * @note Asset type: LSAT_BULLETSHAPE
 * @note More expensive than convex shapes - use only for static geometry
 * 
 * @example
 * @code
 * // Create static terrain collision
 * lsAsset* meshAsset = am->Add_Asset(LSAT_MESH, "meshes/terrain.glb");
 * lsAsset* shape = am->Add_Asset(LSAT_BULLETSHAPE_TRIMESH);
 * // Use for static object only
 * @endcode
 */
class lsAsset_BulletShape_TriMesh : public lsAsset
{
public:
	/**
	 * @brief Construct triangle mesh from mesh asset
	 * @param meshAsset Mesh to create shape from
	 */
    lsAsset_BulletShape_TriMesh(lsAsset_Mesh* meshAsset);
    
    /// Destructor
    virtual ~lsAsset_BulletShape_TriMesh();

    //=========================================================================
    // lsAsset Override
    //=========================================================================
    
    /**
     * @brief Create triangle mesh shape
     */
    void AllocAsset() override;
    
    /**
     * @brief Destroy shape
     */
    void FreeAsset() override;

private:
    /// Source mesh for triangle mesh
    lsAsset_Mesh* m_meshAsset;
};
