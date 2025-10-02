// vortxGE
#include "../vortxGE.h"

// datablocks
#include "../datablocks/lsData.h"
#include "../datablocks/lsMesh.h"
#include "../datablocks/lsStats.h"

#include "lsAsset_Mesh.h"

/*! Mesh Asset
 * Asset Manager sould creat this instance.
 * */
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
		std::string ext;
		size_t pos = filename.find_last_of(".");
		if (pos != std::string::npos) {
			ext = filename.substr(pos + 1);
		}
		
		std::cout << "INFO: Loading mesh '" << filename << "' with extension '" << ext << "'" << std::endl;
		
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

