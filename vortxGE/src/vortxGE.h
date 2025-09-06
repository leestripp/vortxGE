#ifndef vortxGE_vortxGE_h
#define vortxGE_vortxGE_h

#include <iostream>
#include <string>

class lsStats;
class lsOpenGL;
class lsShaderManager;
class lsControlManager;
class lsAssetManager;

using namespace std;

//! Graphics Engine Core
/*!
 * Create an instance of this class in your app.
 * You call Init() once you have a working openGL context.
 * Then in a loop, call Update() and Render().
 *
 * Init sets up the following instances.
 * lsOpenGL
 * lsShaderManager  : also passed to lsOpenGL as ref
 * lsStats          : also passed to lsOpenGL
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

    lsOpenGL* getOpenGL() const
    {
        return opengl;
    }

private:
    // Managers
    lsOpenGL            *opengl;
    lsShaderManager     *shadermanager;
    lsAssetManager      *assetmanager;
    lsControlManager    *controlmanager;
    lsStats             *stats;

};

#endif
