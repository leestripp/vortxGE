/**
 * @file lsAsset_Mesh.h
 * @brief Mesh asset for loading 3D geometry
 * 
 * Asset type for loading and managing 3D mesh data from files.
 * Supports various mesh formats through the asset manager.
 * 
 * @author vortxGE
 * @date 2024
 * @see lsAsset
 * @see lsAssetManager
 */

#pragma once

#include "lsAsset.h"

class lsShaderManager;
class lsAsyncFileLoader;

#include <future>
#include <expected>
#include "../datablocks/lsGLTFParser.h"

/**
 * @class lsAsset_Mesh
 * @brief Asset for loading 3D mesh geometry
 * 
 * Represents a 3D mesh that can be loaded from file and used
 * by the rendering system. The actual mesh data is stored in
 * GPU buffers after loading.
 * 
 * @note Asset type: LSAT_MESH
 * 
 * @example
 * @code
 * lsAssetManager* am = scene->Get_AssetManager();
 * lsAsset* meshAsset = am->Add_Asset(LSAT_MESH, "meshes/cube.glb");
 * @endcode
 */
class lsAsset_Mesh : public lsAsset
{
public:
	/**
	 * @brief Construct mesh asset
	 * @param render Pointer to renderer
	 */
    lsAsset_Mesh( lsRender *render );
    
    /// Destructor
    virtual ~lsAsset_Mesh();

    //=========================================================================
    // lsAsset Override
    //=========================================================================
    
    /**
     * @brief Allocate mesh data from file
     * 
     * Loads mesh data from the file specified by getFilename()
     * into GPU memory.
     */
    void AllocAsset();
    
    /**
     * @brief Free mesh data
     * 
     * Releases GPU memory used by the mesh.
     */
    void FreeAsset();

    /** M5: queue load+ParseGLTF on the loader's workers; AsyncPoll()
     *  runs UploadGLTF on the calling (GL) thread. */
    void AllocAsset_Async( lsAsyncFileLoader& loader ) override;
    bool AsyncPoll() override;

    //=========================================================================
    // Properties
    //=========================================================================
    
    /**
     * @brief Get mesh filename
     * @return Path to mesh file
     */
    const string& getFilename() const
    {
        return filename;
    }

    /**
     * @brief Set mesh filename
     * @param val Path to mesh file
     */
    void setFilename( const string& val )
    {
        filename = val;
    }

private:
    /// Path to mesh file
    string filename;

    std::future<std::expected<lsGLTFMeshData, std::string>> m_loadFuture;
    bool m_asyncLoading = false;
};
