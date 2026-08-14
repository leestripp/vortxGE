/**
 * @file lsBatch_Bullet.h
 * @brief Physics mesh batch for Bullet physics integration
 * 
 * lsBatch_Bullet creates triangle mesh geometries that can be used
 * with the Bullet physics engine. It supports two types of physics shapes:
 * 
 * @section shape_types Shape Types
 * 
 * - Convex Hull: Wraps the mesh in a bounding convex shape
 * - Triangle Mesh (BVH): Exact mesh geometry for static objects
 * 
 * @section usage Usage
 * 
 * @code
 * lsBatch_Bullet* bullet = new lsBatch_Bullet(renderer);
 * bullet->Begin(maxVerts);
 * // Add vertices and triangles
 * bullet->AddTriangle(verts);
 * btBvhTriangleMeshShape* shape = bullet->End_TriMesh(scale);
 * // Or for convex objects:
 * btConvexHullShape* hull = bullet->End_ConvexHull();
 * @endcode
 * 
 * @note Triangle mesh shapes are for static objects (world geometry)
 * @note Convex hull shapes are for dynamic objects
 * 
 * @see lsBatch for base class
 * @see lsBulletWorld for physics world
 * @see lsAsset_BulletShape_TriMesh for triangle mesh asset
 * @see lsAsset_BulletShape_ConvexHull for convex hull asset
 */

#pragma once

#include "lsBatch.h"

#include "../render/lsRender.h"

// math
#include "../math/lsVector2.h"
#include "../math/lsVector3.h"
#include "../math/lsVector4.h"


// Forward declarations
class btBvhTriangleMeshShape;
class btConvexHullShape;


/**
 * @class lsBatch_Bullet
 * @brief Physics mesh batch for Bullet physics
 * 
 * lsBatch_Bullet creates physics-ready triangle meshes for use
 * with the Bullet physics engine. It can generate either BVH triangle
 * meshes (for static collision) or convex hull shapes (for dynamic).
 * 
 * @note BVH shapes are efficient for static world geometry
 * @note Convex hulls are required for moving rigid bodies
 */

class lsBatch_Bullet : public lsBatch
{
public:
    /**
     * @brief Construct a Bullet physics batch
     * @param render Pointer to the renderer
     */
	lsBatch_Bullet( lsRender *render );
	
    /**
     * @brief Destructor
     */
	virtual ~lsBatch_Bullet();

    //=========================================================================
    // lsBatch Virtual Methods
    //=========================================================================
	
    /**
     * @brief Draw the physics mesh
     * 
     * Renders wireframe representation of the physics mesh.
     * 
     * @param cam Camera for rendering
     * @param object Object being rendered
     * @param pass Current render pass
     */
    void Draw( lsCamera *cam, lsObject *object, int pass_type );
    
    /**
     * @brief Update batch state
     * 
     * @param delta Time since last frame
     */
    void Update( double delta );

	//=========================================================================
    // Geometry Building
    //=========================================================================
	
    /**
     * @brief Begin geometry construction
     * 
     * @param nMaxVerts Maximum number of vertices to allocate
     */
	void Begin( lsUInt nMaxVerts );
	
    /**
     * @brief Add a single vertex point
     * 
     * @param vert Vertex position
     */
	void AddPoint( Vector3f vert );
	
    /**
     * @brief Add a triangle
     * 
     * @param verts Array of 3 vertex positions
     */
	void AddTriangle( Vector3f verts[3] );
	
    /**
     * @brief Create BVH triangle mesh shape
     * 
     * Creates a Bullet BVH (Bounding Volume Hierarchy) triangle mesh
     * shape for static collision detection.
     * 
     * @param size Scale factor
     * @return Pointer to btBvhTriangleMeshShape
     */
	btBvhTriangleMeshShape*  End_TriMesh( float size );
	
    /**
     * @brief Create convex hull shape
     * 
     * Creates a Bullet convex hull shape for dynamic rigid bodies.
     * 
     * @return Pointer to btConvexHullShape
     */
	btConvexHullShape*  End_ConvexHull();
	
    /**
     * @brief Clean up temporary data
     */
	void Cleanup();

	//=========================================================================
    // OpenGL Data Management
    //=========================================================================
	
    /**
     * @brief Upload data to GPU
     */
    void Copy_GLdata();
    
    /**
     * @brief Release GPU resources
     */
    void Clear_GLdata();
	
	//=========================================================================
    // Utilities
    //=========================================================================
	
    /**
     * @brief Check if two floats are close enough
     * 
     * @param fCandidate First value
     * @param fCompare Second value
     * @param fEpsilon Comparison tolerance
     * @return true if values are within epsilon
     */
	bool CloseEnough( const float fCandidate, const float fCompare, const float fEpsilon );
	
private:
    /// Maximum number of indices
	lsUInt nMaxIndexes;
	
    /// Current index count
	lsUInt nNumIndexes;
	
    /// Current vertex count
	lsUInt nNumVerts;
	
    //=========================================================================
    // Host Data
    //=========================================================================
	
    /** @brief Index array */
	std::uint16_t *pIndexes;
	
    /** @brief Vertex array */
	Vector3f *pVerts;
};
