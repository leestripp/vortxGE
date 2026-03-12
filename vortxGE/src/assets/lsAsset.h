/**
 * @file lsAsset.h
 * @brief Base class for all engine assets (textures, meshes, physics shapes)
 * 
 * lsAsset is the base class for all loadable resources in the engine.
 * It provides:
 * - Asset type identification
 * - Reference counting (user count)
 * - Linked-list navigation for asset manager
 * - Virtual allocation/deallocation methods
 * 
 * @section asset_types Asset Types
 * 
 * Supported asset types (LSAT_*):
 * - LSAT_NONE: Undefined
 * - LSAT_TEXTURE: Image textures
 * - LSAT_MESH: 3D meshes
 * - LSAT_QUAD: 2D quad for UI/post-processing
 * - LSAT_BULLETSHAPE_*: Physics collision shapes
 * - LSAT_MESHGUIDE_*: Editor visualization guides
 * 
 * @section reference_counting Reference Counting
 * 
 * Assets use reference counting to manage lifetime.
 * - AddUser() increments count when asset is in use
 * - RemoveUser() decrements count when done
 * - Asset can be unloaded when users == 0
 * 
 * @see lsAssetManager for asset loading and caching
 * @see lsAsset_Mesh for mesh assets
 * @see lsAsset_Texture for texture assets
 */

#pragma once

#include <iostream>
#include <string>

// vortxGE
#include "../vortx_debug.h"

// Render
#include "../render/lsRender.h"
#include "../render/lsOpenGL.h"

//=========================================================================
// Asset Type Constants
//=========================================================================

/** @brief Undefined asset type */
const int LSAT_NONE                 = 0;
/** @brief Image texture */
const int LSAT_TEXTURE              = 1;
/** @brief 3D mesh geometry */
const int LSAT_MESH                 = 2;
/** @brief Physics shape: infinite plane */
const int LSAT_BULLETSHAPE_PLANE    = 3;
/** @brief Physics shape: box */
const int LSAT_BULLETSHAPE_BOX      = 4;
/** @brief Physics shape: sphere */
const int LSAT_BULLETSHAPE_SPHERE   = 5;
/** @brief Physics shape: capsule (pill) */
const int LSAT_BULLETSHAPE_PILL     = 6;
/** @brief 2D quad for rendering */
const int LSAT_QUAD                 = 7;
/** @brief Physics shape: convex hull */
const int LSAT_BULLETSHAPE_CONVEXHULL = 8;
/** @brief Physics shape: triangle mesh */
const int LSAT_BULLETSHAPE_TRIMESH    = 9;
// Editor types
/** @brief Editor: empty placeholder */
const int LSAT_MESHGUIDE_EMPTY      = 1000;
/** @brief Editor: grid visualization */
const int LSAT_MESHGUIDE_GRID       = 1001;

// Forward declarations
class lsAssetManager;
class lsData;

/**
 * @class lsAsset
 * @brief Base class for all loadable assets
 * 
 * Provides common functionality for all asset types:
 * - Type identification
 * - Reference counting
 * - Navigation (linked list)
 * - Data storage interface
 * 
 * @section lifecycle Lifecycle
 * 
 * Typical asset lifecycle:
 * 1. lsAssetManager loads asset from disk
 * 2. Asset is allocated (AllocAsset)
 * 3. User counts incremented (AddUser)
 * 4. Asset is used for rendering/physics
 * 5. User decrements count (RemoveUser)
 * 6. When users == 0, asset may be unloaded
 */
class lsAsset
{
public:
    /**
     * @brief Construct an asset
     * @param m_render Pointer to renderer
     */
    lsAsset( lsRender *m_render );
    
    /**
     * @brief Destructor
     * 
     * Frees asset data if allocated.
     */
    virtual ~lsAsset();

    //=========================================================================
    // Reference Counting
    //=========================================================================
    
    /**
     * @brief Increment user count
     * 
     * Called when something starts using this asset.
     * Prevents asset from being unloaded while in use.
     */
    void AddUser();
    
    /**
     * @brief Decrement user count
     * 
     * Called when something stops using this asset.
     * When count reaches 0, asset can be unloaded.
     */
    void RemoveUser();

    //=========================================================================
    // Virtual Methods
    //=========================================================================
    
    /**
     * @brief Allocate GPU resources
     * 
     * Pure virtual - subclasses implement to upload
     * data to GPU, create OpenGL objects, etc.
     */
    virtual void AllocAsset() = 0;
    
    /**
     * @brief Free GPU resources
     * 
     * Pure virtual - subclasses implement to release
     * GPU memory and OpenGL objects.
     */
    virtual void FreeAsset() = 0;

    //=========================================================================
    // Navigation
    //=========================================================================
    
    /**
     * @brief Get next asset in list
     * @return Pointer to next asset
     */
    lsAsset *getNext_asset() const
    {
        return next_asset;
    }

    /**
     * @brief Set next asset
     * @param next_asset Pointer to next asset
     */
    void setNext_asset(lsAsset *next_asset)
    {
        this->next_asset = next_asset;
    }

    /**
     * @brief Get previous asset in list
     * @return Pointer to previous asset
     */
    lsAsset *getPrev_asset() const
    {
        return prev_asset;
    }

    /**
     * @brief Set previous asset
     * @param prev_asset Pointer to previous asset
     */
    void setPrev_asset(lsAsset *prev_asset)
    {
        this->prev_asset = prev_asset;
    }

    //=========================================================================
    // Accessors
    //=========================================================================
    
    /**
     * @brief Get asset type
     * @return Asset type constant (LSAT_*)
     */
    int getAsset_type() const
    {
        return asset_type;
    }

    /**
     * @brief Set asset type
     * @param val Asset type constant
     */
    void setAsset_type( int val )
    {
        asset_type = val;
    }

    /**
     * @brief Get asset name
     * @return Name string (usually filename)
     */
    const string& getAsset_name()
    {
        return asset_name;
    }

    /**
     * @brief Set asset name
     * @param name New name
     */
    void setAsset_name( const string& name )
    {
        asset_name = name;
    }

    /**
     * @brief Get user count
     * @return Number of users currently using this asset
     */
    unsigned int getUsers() const
    {
        return users;
    }

    /**
     * @brief Get associated data
     * @return Pointer to lsData object
     */
    lsData* getData() const
    {
        return data;
    }

    /**
     * @brief Set associated data
     * @param val Pointer to lsData
     */
    void setData( lsData* val )
    {
        data = val;
    }

    //=========================================================================
    // Manager
    //=========================================================================
    
    /**
     * @brief Get asset manager
     * @return Pointer to managing lsAssetManager
     */
    lsAssetManager* getAssetManager() const
    {
        return assetmanager;
    }

    /**
     * @brief Set asset manager
     * @param val Pointer to manager
     */
    void setAssetManager( lsAssetManager* val )
    {
        assetmanager = val;
    }

    //=========================================================================
    // Render
    //=========================================================================
    
    /**
     * @brief Get renderer
     * @return Pointer to lsRender
     */
    lsRender* getOpenGL() const
    {
        return render;
    }

    /**
     * @brief Set renderer
     * @param val Pointer to renderer
     */
    void setOpenGL( lsRender* val )
    {
        render = val;
    }

private:
    //=========================================================================
	// Private Members
	//=========================================================================
	
    /** @brief Next asset in linked list */
    lsAsset* next_asset;
    
    /** @brief Previous asset in linked list */
    lsAsset* prev_asset;
    
    /** @brief Asset manager that owns this asset */
    lsAssetManager *assetmanager;
    
    /** @brief Renderer reference */
    lsRender *render;

    /** @brief Asset type constant */
    int asset_type;
    
    /** @brief Asset name/identifier */
    string asset_name;
    
    /** @brief Reference count - number of users */
    unsigned int users;
    
    /** @brief Associated data object */
    lsData *data;
};
