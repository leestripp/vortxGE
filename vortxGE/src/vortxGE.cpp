#include <iostream>

// vortx configs
#include "vortx_debug.h"
#include "config.h"

// OpenGL
#include "opengl/lsOpenGL.h"

// OpenAL
#include "openal/lsOpenAL.h" // Include lsOpenAL header

// Controls
#include "controls/lsControlManager.h"

// Shader
#include "shader/lsShaderManager.h"

// Assets
#include "assets/lsAssetManager.h"

// datablocks
#include "datablocks/lsStats.h"

#include "vortxGE.h"

//! Core class of the library
/*!
 * After creating an instance of vortxGE you must create a valid openGL context for output,
 * once openGL is ready then call Init() on the vortxGE instance.
 *
 */
vortxGE::vortxGE()
{
	// Display version
	cout << "NEW instance : vortxGE v" << vortxGE_VERSION_MAJOR;
	cout << "." << vortxGE_VERSION_MINOR;
	cout << "." << vortxGE_VERSION_PATCH;
	cout << " by Lee A. Stripp" << endl;
	
	// Set some defaults, Must call Init() when a valid openGL context is ready.
	opengl = NULL;
	// Managers
	shadermanager = NULL;
	assetmanager = NULL;
	controlmanager = NULL;
	stats = NULL;
	openal = NULL;
}

/*!
 * Users must call cleanup() before deleting.
 */
vortxGE::~vortxGE()
{
}

//*****************************

/*!
 * Init creates instances of :
 *
 *      lsStats
 *      lsOpenGL
 *      lsShaderManager
 *      lsAssetManager
 *      lsControlManager
 *      lsOpenAL
 *
 * Then links the instances of lsStats and lsShaderManager to lsOpenGL
 * Next it calls Init on both lsOpenGL and lsShaderManager
 *
 * @returns 1 = error, 0 = no errors.
 */
int vortxGE::Init()
{
	stats = new lsStats();
	if( stats )
	{
		stats->Init();
	} else
	{
		cout << "ERROR: vortxGE::Init() - Could not instance lsStats" << endl;
		return 1;
	}

	opengl = new lsOpenGL;
	if(! opengl )
	{
		cout << "ERROR: vortxGE::Init() - Could not instance lsOpenGL" << endl;
		delete stats;
		return 1;
	}
	
	shadermanager = new lsShaderManager( opengl );
	if(! shadermanager )
	{
		cout << "ERROR: vortxGE::Init() - Could not instance lsShaderManager" << endl;
		delete stats;
		delete opengl;
		return 1;
	}
	
	assetmanager = new lsAssetManager( opengl );
	if(! assetmanager )
	{
		cout << "ERROR: vortxGE::Init() - Could not instance lsAssetManager" << endl;
		delete stats;
		delete opengl;
		delete shadermanager;
		return 1;
	}
	
	controlmanager = new lsControlManager( opengl );
	if(! controlmanager )
	{
		cout << "ERROR: vortxGE::Init() - Could not instance lsControlManager" << endl;
		delete stats;
		delete opengl;
		delete shadermanager;
		delete assetmanager;
		return 1;
	}
	
	// OpenaL
	openal = new lsOpenAL();
	if(! openal || ! openal->Init())
	{
		cout << "ERROR: vortxGE::Init() - Could not instance or initialize lsOpenAL" << endl;
		delete stats;
		delete opengl;
		delete shadermanager;
		delete assetmanager;
		if (openal) delete openal;
		return 1;
	}
	
	// shared info
	opengl->setStats( stats );
	opengl->setShaderManager( shadermanager );
	
	// Init subsystems
	opengl->Init();
	shadermanager->Init();
	
	opengl->StoreFBO();
	
	return 0;
}

void vortxGE::Cleanup()
{
	if( controlmanager ) delete controlmanager;
	if( assetmanager ) delete assetmanager;
	if( shadermanager ) delete shadermanager;

	if( opengl ) delete opengl;
	if( stats ) delete stats;
	if( openal ) delete openal; // Delete OpenAL instance

	controlmanager = NULL;
	assetmanager = NULL;
	shadermanager = NULL;

	opengl = NULL;
	stats = NULL;
	openal = NULL;
}

/*!
 * Calls Update() on the base Control Manager.
 * This does threaded Move and Optimise on scene nodes to build a draw list based on cull camera and passes.
 * This updates all scene elements, matrix data, physics and animation.
 *
 * \param delta
 */
void vortxGE::Update( double delta )
{
	stats->Loop_Start();

	// Move all active scene objects
	if( controlmanager ) controlmanager->Update( delta );
}

/*!
 * After Update hase created a draw list. Render then draws all objects in that list.
 *
 */
void vortxGE::Render()
{
#ifdef VORTX_DEBUG_RENDER
	cout << endl << "*************************" << endl;
	cout << "vortxGE::Render() - START" << endl;
#endif

	// render - all elements to screen
	if( controlmanager ) controlmanager->Render();

	// reset framebuffer
	opengl->RestoreFBO();

	stats->Loop_Stop();
	stats->Update();

#ifdef VORTX_DEBUG_RENDER
	cout << "vortxGE::Render() - END" << endl;
	cout << "*************************" << endl;
#endif
}

//! Sets the viewport
/*!
 * When your window changes size you should call this function.
 * This updates all passes you have setup.
 *
 * \param width
 * \param height
 */
void vortxGE::Resize( int x, int y, int width, int height )
{
	// pass resize up chain
	if( controlmanager ) controlmanager->Resize( x, y, width, height );
}
