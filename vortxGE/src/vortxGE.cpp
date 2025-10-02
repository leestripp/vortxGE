#include <iostream>

// vortx configs
#include "vortx_debug.h"
#include "config.h"

// Render
#include "render/lsRender.h"
#include "render/lsOpenGL.h"

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
 * After creating an instance of vortxGE you must create a valid render context for output,
 * once render instance is ready then call Init() on the vortxGE instance.
 *
 */
vortxGE::vortxGE()
{
	// Display version
	cout << "NEW instance : vortxGE v" << vortxGE_VERSION_MAJOR;
	cout << "." << vortxGE_VERSION_MINOR;
	cout << "." << vortxGE_VERSION_PATCH;
	cout << " by Lee A. Stripp" << endl;
	
	// Set some defaults, Must call Init() when a valid render context is ready.
	render = nullptr;
	// Managers
	shadermanager = nullptr;
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
 *      lsRender
 *      lsShaderManager
 *      lsAssetManager
 *      lsControlManager
 *      lsOpenAL
 *
 * Then links the instances of lsStats and lsShaderManager to lsRender
 * Next it calls Init on both lsRender and lsShaderManager
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
	
	// TODO: Support more render libries.
	// Currently vortxGE only support OpenGL
	render = new lsOpenGL;
	if(! render )
	{
		cout << "ERROR: vortxGE::Init() - Could not instance lsRender" << endl;
		delete stats;
		return 1;
	}
	
	shadermanager = new lsShaderManager( render );
	if(! shadermanager )
	{
		cout << "ERROR: vortxGE::Init() - Could not instance lsShaderManager" << endl;
		delete stats;
		delete render;
		return 1;
	}
	
	assetmanager = new lsAssetManager( render );
	if(! assetmanager )
	{
		cout << "ERROR: vortxGE::Init() - Could not instance lsAssetManager" << endl;
		delete stats;
		delete render;
		delete shadermanager;
		return 1;
	}
	
	controlmanager = new lsControlManager( render );
	if(! controlmanager )
	{
		cout << "ERROR: vortxGE::Init() - Could not instance lsControlManager" << endl;
		delete stats;
		delete render;
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
		delete render;
		delete shadermanager;
		delete assetmanager;
		if (openal) delete openal;
		return 1;
	}
	
	// shared info
	render->setStats( stats );
	render->setShaderManager( shadermanager );
	
	// Init subsystems
	render->Init();
	shadermanager->Init();
	
	// Desktops Framebuffer
	render->StoreDefaultFramebuffer();
	
	return 0;
}

void vortxGE::Cleanup()
{
	if( controlmanager ) delete controlmanager;
	if( assetmanager ) delete assetmanager;
	if( shadermanager ) delete shadermanager;
	
	if( render ) delete render;
	if( stats ) delete stats;
	if( openal ) delete openal;
	
	controlmanager = NULL;
	assetmanager = NULL;
	shadermanager = NULL;
	
	render = NULL;
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
	render->RestoreDefaultFramebuffer();
	
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
