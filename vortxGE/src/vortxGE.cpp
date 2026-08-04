#include <iostream>

// vortx configs
#include "vortx_debug.h"
#include "config.h"

// Render
#include "render/lsRender.h"
#include "render/lsOpenGL.h"
#include "render/lsVulkan.h"

#include <cstdlib>		// getenv - VORTX_RENDERER selection

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
	
	// Renderer selection (Plan.md Part 2): Vulkan is the primary backend,
	// OpenGL the fallback. ForceRenderer() overrides; otherwise env/probe
	// (same logic as PreferredRendererType — the app used that result to
	// create the window, so both paths agree).
	{
		RenderType want = (m_forcedRenderer != RenderType::None)
			? m_forcedRenderer : PreferredRendererType();

		if( want == RenderType::Vulkan )
		{
			render = new lsVulkan;
			if( render )
			{
				render->SetWindow( m_nativeWindow );
				if( render->Init() )
				{
					cout << "INFO: vortxGE::Init() - using Vulkan renderer" << endl;
				} else
				{
					// Probe said OK but real init failed — fall back to GL.
					// NOTE: if the window was created GLFW_NO_API this GL
					// fallback will not be able to render; log loudly.
					cout << "ERROR: vortxGE::Init() - Vulkan init failed after a "
							"successful probe; falling back to OpenGL (window may "
							"have been created without a GL context!)" << endl;
					delete render;
					render = NULL;
				}
			}
		}
		if(! render )
		{
			render = new lsOpenGL;
			if( render ) cout << "INFO: vortxGE::Init() - using OpenGL renderer" << endl;
		}
	}
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

	// Frame begin (Vulkan: acquire + start recording; GL: no-op)
	if( render ) render->BeginFrame();

	// render - all elements to screen
	if( controlmanager ) controlmanager->Render();

	// reset framebuffer
	render->RestoreDefaultFramebuffer();

	// Frame end + present (Vulkan: submit + present; GL: no-op - the app
	// still calls glfwSwapBuffers until present unification in V6)
	if( render ) { render->EndFrame(); render->Present(); }

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

// ---------------------------------------------------------------------
// Renderer backend selection
// ---------------------------------------------------------------------
RenderType vortxGE::PreferredRendererType()
{
	const char* env = getenv( "VORTX_RENDERER" );
	const string want = env ? env : "vulkan";

	if( want == "opengl" )
		return RenderType::OpenGL;

	// Default/"vulkan": probe the loader + a physical device so the app
	// can create the correct window type before committing to Vulkan.
	if( lsVulkan::Probe() )
		return RenderType::Vulkan;

	cout << "INFO: vortxGE::PreferredRendererType() - no Vulkan device, "
			"OpenGL selected" << endl;
	return RenderType::OpenGL;
}
