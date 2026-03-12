/**
 * @file lsAssetManager.h
 * @brief Asset loading and caching manager
 * 
 * lsAssetManager handles loading and caching of all engine assets:
 * - Meshes (from VBM/GLTF files)
 * - Textures (images)
 * - Physics shapes (collision geometry)
 * - Editor guides (grid, empty)
 * 
 * @section caching Caching
 * 
 * Assets are cached by name. Loading the same asset
 * twice returns the cached version.
 * 
 * @section usage Usage
 * @code
 * lsAssetManager* am = new lsAssetManager(render);
 * 
 * // Load mesh (cached)
 * lsAsset_Mesh* mesh = am->CreateAsset_Mesh("cube.vbm");
 * 
 * // Load texture (cached)
 * lsAsset_Texture* tex = am->CreateAsset_Texture("wood.png");
 * 
 * // Create physics shape
 * lsAsset_BulletShape_Box* box = am->CreateAsset_BulletShape_Box(1, 1, 1);
 * @endcode
 * 
 * @see lsAsset for base asset class
 * @see lsAsset_Mesh for mesh assets
 * @see lsAsset_Texture for texture assets
 */

#pragma once

#include <iostream>
#include <string>

// render
#include "../render/lsRender.h"

// Forward declarations
class lsAsset;
class lsAsset_Mesh;
class lsAsset_Quad;
class lsAsset_MeshGuide_Empty;
class lsAsset_MeshGuide_Grid;
class lsAsset_Texture;
class lsAsset_BulletShape_Plane;
class lsAsset_BulletShape_Box;
class lsAsset_BulletShape_Sphere;
class lsAsset_BulletShape_Pill;
class lsAsset_BulletShape_ConvexHull;
class lsAsset_BulletShape_TriMesh;

/**
 * @class lsAssetManager
 * @brief Manages loading and caching of all assets
 * 
 * Central asset registry that loads assets on demand
 * and caches them for reuse.
 * 
 * @section asset_types Supported Asset Types
 * 
 * - Meshes: CreateAsset_Mesh()
 * - Quads: CreateAsset_Quad()
 * - Textures: CreateAsset_Texture()
 * - Physics: CreateAsset_BulletShape_*()
 * - Guides: CreateAsset_MeshGuide_*()
 */
class lsAssetManager
{
public:
    /**
     * @brief Construct asset manager
     * @param render Pointer to renderer
     */
    lsAssetManager( lsRender *render );
    
    /**
     * @brief Destructor
     */
    virtual ~lsAssetManager();

	//=========================================================================
	// Asset Lookup
	//=========================================================================
	
    /**
     * @brief Find cached asset by name
     * 
     * @param name Asset name (usually filename)
     * @return Pointer to asset, or nullptr if not found
     */
    lsAsset* Find_Asset( const string& name );

    //=========================================================================
	// Mesh Assets
	//=========================================================================
	
    /**
     * @brief Load mesh from file
     * 
     * Supports VBM and GLTF formats.
     * 
     * @param filename Mesh file path
     * @return Pointer to mesh asset (cached)
     */
    lsAsset_Mesh* CreateAsset_Mesh( const string& filename );
    
    /**
     * @brief Create a 2D quad
     * @return Pointer to quad asset
     */
    lsAsset_Quad* CreateAsset_Quad();
    
    //=========================================================================
	// Texture Assets
	//=========================================================================
    
    /**
     * @brief Load texture from image file
     * 
     * Supports common image formats.
     * 
     * @param filename Image file path
     * @return Pointer to texture asset (cached)
     */
    lsAsset_Texture* CreateAsset_Texture( const string& filename );
    
    //=========================================================================
	// Physics Shape Assets
	//=========================================================================
    
    /**
     * @brief Create infinite plane physics shape
     * @param vectorX Plane normal X
     * @param vectorY Plane normal Y
     * @param vectorZ Plane normal Z
     * @return Pointer to plane shape
     */
    lsAsset_BulletShape_Plane* CreateAsset_BulletShape_Plane( float vectorX, float vectorY, float vectorZ );
    
    /**
     * @brief Create box physics shape
     * @param x Half-extent X
     * @param y Half-extent Y
     * @param z Half-extent Z
     * @return Pointer to box shape
     */
    lsAsset_BulletShape_Box* CreateAsset_BulletShape_Box( float x, float y, float z );
    
    /**
     * @brief Create capsule/pill physics shape
     * @param radius Capsule radius
     * @param height Capsule height
     * @return Pointer to pill shape
     */
    lsAsset_BulletShape_Pill* CreateAsset_BulletShape_Pill( float radius, float height );
    
    /**
     * @brief Create convex hull from mesh
     * @param meshAsset Source mesh asset
     * @return Pointer to convex hull shape
     */
    lsAsset_BulletShape_ConvexHull* CreateAsset_BulletShape_ConvexHull(lsAsset_Mesh* meshAsset);
    
    /**
     * @brief Create triangle mesh from mesh
     * @param meshAsset Source mesh asset
     * @return Pointer to tri mesh shape
     */
    lsAsset_BulletShape_TriMesh* CreateAsset_BulletShape_TriMesh(lsAsset_Mesh* meshAsset);
    
    //=========================================================================
	// Editor Guide Assets
	//=========================================================================
    
    /**
     * @brief Create empty placeholder guide
     * @return Pointer to empty guide
     */
    lsAsset_MeshGuide_Empty* CreateAsset_MeshGuide_Empty();
    
    /**
     * @brief Create grid visualization guide
     * @return Pointer to grid guide
     */
    lsAsset_MeshGuide_Grid* CreateAsset_MeshGuide_Grid();

    //=========================================================================
	// Accessors
	//=========================================================================
	
    /**
     * @brief Get loaded asset count
     * @return Number of cached assets
     */
    unsigned int getAsset_count() const
    {
        return asset_count;
    }

    /**
     * @brief Get asset list head
     * @return Pointer to first asset
     */
    lsAsset* getAssetList() const
    {
        return asset_list;
    }

    /**
     * @brief Set renderer
     * @param render Pointer to renderer
     */
    void setOpenGL( lsRender *render )
    {
        m_render = render;
    }

    /**
     * @brief Get renderer
     * @return Pointer to renderer
     */
    lsRender *getOpenGL() const
    {
        return m_render;
    }

private:
    //=========================================================================
	// Private Methods
	//=========================================================================
	
    /**
     * @brief Add asset to cache
     * @param asset Asset to add
     */
    void Add_Asset( lsAsset *asset );
    
    /**
     * @brief Remove asset from cache
     * @param asset Asset to remove
     */
    void Del_Asset( lsAsset* asset );

    //=========================================================================
	// Private Members
	//=========================================================================
	
    /** @brief Head of asset linked list */
    lsAsset         *asset_list;
    
    /** @brief Number of cached assets */
    unsigned int 	asset_count;

    /** @brief Renderer reference */
    lsRender *m_render;

};
