#pragma once

#include <iostream>
#include <string>

class lsStats;
class lsRender;
class lsShaderManager;
class lsControlManager;
class lsAssetManager;
class lsOpenAL;

using namespace std;

//! Graphics Engine Core
/*!
 * Create an instance of this class in your app.
 * You call Init() once you have a working render context.
 * Then in a loop, call Update() and Render().
 *
 * Init sets up the following instances.
 * lsRender
 * lsShaderManager  : also passed to lsRender as ref
 * lsStats          : also passed to lsRender
 * lsAssetManager   : All lsControls have their own Asset Manager, this one is only used for global assets.
 * lsControlManager : Viewer, Menus, Scene, Gameplay Controls
 *
 */
class vortxGE
{
public:
	vortxGE();
	virtual ~vortxGE();
	
	int Init();
	void Cleanup();
	void Resize( int x, int y, int width, int height );
	
	// Main loop
	void Update( double delta );
	void Render();
	
	lsControlManager* getControlManager() const
	{
		return controlmanager;
	}

	lsShaderManager* getShaderManager() const
	{
		return shadermanager;
	}

	lsAssetManager* getAssetManager() const
	{
		return assetmanager;
	}
	
	lsRender* getRender() const
	{
		return render;
	}
	
	lsOpenAL* getOpenAL() const
	{
		return openal;
	}
	
private:
	// Managers
	lsRender            *render;
	lsOpenAL            *openal;
	lsShaderManager     *shadermanager;
	lsAssetManager      *assetmanager;
	lsControlManager    *controlmanager;
	lsStats             *stats;
};
