/**
 * @file lsAsset_Mesh.cpp
 * @brief Mesh asset implementation
 * 
 * This file implements the mesh asset loader which handles
 * loading 3D mesh data from various file formats.
 * 
 * Supported formats:
 * - VBM: Custom vertex buffer mesh format
 * - GLTF/GLB: glTF 2.0 format (embedded or binary)
 * 
 * The asset manager creates this asset type when loading
 * mesh files. The actual loading is delegated to lsMesh.
 * 
 * @see lsAsset.h
 * @see lsAsset_Mesh.h
 * @see lsMesh.h
 */

// vortxGE
#include "../vortxGE.h"

// datablocks
#include "../datablocks/lsData.h"
#include "../datablocks/lsMesh.h"
#include "../datablocks/lsStats.h"

#include "lsAsset_Mesh.h"

/**
 * @brief Construct a mesh asset
 * 
 * @param render Pointer to renderer
 */
lsAsset_Mesh::lsAsset_Mesh( lsRender *render ) : lsAsset(render)
{
	setAsset_type( LSAT_MESH );
	setAsset_name( "Mesh" );
}

lsAsset_Mesh::~lsAsset_Mesh()
{
	FreeAsset();
}

// #######################
// AllocAsset

/**
 * @brief Allocate and load mesh asset
 * 
 * This method loads mesh data from file based on extension:
 * 
 * Supported formats:
 * - .vbm: Vertex Buffer Mesh (custom format)
 * - .gltf: glTF JSON format
 * - .glb: glTF binary format
 * 
 * Loading process:
 * 1. Extract file extension from filename
 * 2. Create lsMesh object
 * 3. Call appropriate loader based on format
 * 4. Store mesh data in asset
 * 5. Update memory statistics
 * 
 * @note Loads vertex data, normals, UVs, and indices
 * @note Also loads embedded textures for GLTF
 */

void lsAsset_Mesh::AllocAsset()
{
#ifdef VORTX_DEBUG_ASSETS
	cout << "lsAsset_Mesh::AllocAsset() for file: " << filename << endl;
#endif
	
	lsMesh *mesh;
	
	// Clear old data
	if( getData() )
	{
		delete getData();
		setData( NULL );
	}
	if(! getOpenGL() ) return;
	
	mesh = new lsMesh( getOpenGL() );
	if( mesh )
	{
		// Check file extension
		string ext;
		size_t pos = filename.find_last_of(".");
		if (pos != string::npos) {
			ext = filename.substr(pos + 1);
		}
		
		cout << "INFO: Loading mesh '" << filename << "' with extension '" << ext << "'" << endl;
		
		if (ext == "vbm")
		{
			mesh->LoadVBM( filename, getAssetManager(), getOpenGL()->getShaderManager() );
		} else if (ext == "gltf" || ext == "glb")
		{
			mesh->LoadGLTF( filename, getAssetManager(), getOpenGL()->getShaderManager() );
		} else
		{
			cout << "ERROR: lsAsset_Mesh::AllocAsset() - Unsupported mesh format: " << ext << endl;
			delete mesh;
			return;
		}
		
		setData( mesh );
		// memory used
		getOpenGL()->getStats()->addUsed_mem( mesh->getMem_used() );
		
	} else
	{
		// Error
		cout << "ERROR: Could not create lsMesh instance - lsAsset_Mesh::AllocAsset()" << endl;
	}
}

/**
 * @brief Free mesh asset memory
 * 
 * Releases the mesh data and updates memory statistics.
 * This is called when the asset is unloaded or destroyed.
 * 
 * Cleanup process:
 * 1. Subtract mesh memory usage from stats
 * 2. Delete the lsMesh object
 * 3. Clear data pointer
 */

void lsAsset_Mesh::FreeAsset()
{
	if( getData() )
	{
		// memory used
		getOpenGL()->getStats()->addUsed_mem( -getData()->getMem_used() );
		
		delete getData();
		setData( NULL );
	}
}

