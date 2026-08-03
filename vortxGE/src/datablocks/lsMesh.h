/**
 * @file lsMesh.h
 * @brief Mesh data container
 * 
 * Container class for mesh geometry, managing batches of
 * renderable geometry.
 * 
 * @author vortxGE
 * @date 2024
 * @see lsBatch
 * @see lsAsset_Mesh
 */

#pragma once

#include <iostream>
#include <string>

// math
#include "../math/lsBoundingSphere.h"

#include "lsData.h"

// Forward declarations
class lsBatch;
class lsObject;
class lsCamera;
class lsPass;
class lsRender;
class lsAssetManager;
class lsShaderManager;

using namespace std;

/**
 * @class lsMesh
 * @brief Mesh geometry container
 * 
 * Manages a list of lsBatch objects that make up a complete mesh.
 * Each batch represents a draw call with specific shader and material.
 * 
 * Supported formats:
 * - VBM (vortxGE native)
 * - glTF (standard 3D format)
 * 
 * @note Inherits from lsData for memory tracking
 * 
 * @example
 * @code
 * lsMesh* mesh = new lsMesh(renderer);
 * mesh->LoadGLTF("models/cube.glb", assetManager, shaderManager);
 * 
 * // In render loop
 * mesh->Draw(camera, pass, object);
 * @endcode
 */
class lsMesh : public lsData
{
public:
	/**
	 * @brief Construct mesh
	 * @param render Pointer to renderer
	 */
	lsMesh( lsRender *render );
	
	/// Destructor
	virtual ~lsMesh();

    //=========================================================================
    // Rendering
    //=========================================================================
    
    /**
     * @brief Draw mesh
     * 
     * @param cam Camera
     * @param pass Render pass
     * @param object Object being rendered
     */
	void Draw( lsCamera *cam, lsPass *pass, lsObject *object );
	
    /**
     * @brief Update mesh animation
     * 
     * Advances skeletal animation if mesh has bones.
     */
	void Update_Animation();

    //=========================================================================
    // Loading
    //=========================================================================
    
    /**
     * @brief Load VBM mesh
     * 
     * @param filename Path to VBM file
     * @param am Asset manager
     * @param sm Shader manager
     */
	
    /**
     * @brief Load glTF mesh
     * 
     * @param filename Path to glTF/glb file
     * @param am Asset manager
     * @param sm Shader manager
     */
	void LoadGLTF( const string& filename, lsAssetManager *am, lsShaderManager *sm );

	/** GL upload from CPU-parsed data (M5): creates batches, textures,
	 *  shaders and the bounding sphere. LoadGLTF() = file read + ParseGLTF
	 *  + UploadGLTF; the async pipeline parses off-thread and calls this
	 *  on the GL thread. */
	void UploadGLTF( const struct lsGLTFMeshData& data, lsAssetManager *am, lsShaderManager *sm );
	
    /**
     * @brief Add batch to mesh
     * 
     * @param batch Batch to add
     */
	void Add_Batch( lsBatch* batch );

    //=========================================================================
    // Accessors
    //=========================================================================
	
    /**
     * @brief Get bounding sphere
     * @return Bounding sphere encompassing all batches
     */
	const lsBoundingSphere& getBSphere()
	{
		return bSphere;
	}

	/**
	 * @brief Get batch list
	 * @return First batch in list
	 */
	lsBatch* getBatch_list() const
	{
		return batch_list;
	}

	/**
	 * @brief Get renderer
	 * @return Pointer to renderer
	 */
	lsRender *getRender() const;
	
	/**
	 * @brief Set renderer
	 * @param value Pointer to renderer
	 */
	void setOgl(lsRender *value);

private:
	/// List of batches making up this mesh
	lsBatch* batch_list;

	/// Bounding sphere encompassing all batches
	lsBoundingSphere bSphere;
	
	/// Renderer reference
	lsRender *m_render;

};
