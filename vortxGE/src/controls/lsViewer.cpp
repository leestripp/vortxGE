#include <iostream>
// debug
#include "../vortx_debug.h"

#include <iostream>

#include "../assets/lsAssetManager.h"
#include "../scene/lsSceneManager.h"
#include "../passes/lsPassManager.h"
#include "lsControlManager.h"

#include "lsViewer.h"

lsViewer::lsViewer( lsRender *render, lsControl *p ) : lsControl(render,p)
{
    setControl_type( LSCT_VIEWER );
    setName("Viewer");
    setActive( true );

    // managers : call Init() to setup
    scenemanager = NULL;
    passmanager = NULL;
    assetmanager = NULL;

}

lsViewer::~lsViewer()
{
    if( passmanager ) delete passmanager;
    if( scenemanager ) delete scenemanager;
    if( assetmanager ) delete assetmanager;
}

/*!
 * \brief lsViewer::Init
 *
 * Creates the following instances :\n
 * lsAssetManager   : local assets for this control.\n
 * lsSceneManager\n
 * lsPassManager\n
 * \n
 *
 * \param render - link to the lsRender instance created in vortxGE
 * \return - error code: not implemented yet.
 */
int lsViewer::Init()
{
    // Asset Manager
    assetmanager = new lsAssetManager( getRender() );
    if(! assetmanager )
    {
        // ERROR
        cout << "ERROR: lsViewer::Init - Could not create Asset Manager instance" << endl;
    }

    scenemanager = new lsSceneManager();
    if(! scenemanager )
    {
        // ERROR
        cout << "ERROR: lsViewer::Init - Could not create Scene Manager instance" << endl;
    }

    passmanager = new lsPassManager( getRender() );
    if(! passmanager )
    {
        // ERROR
        cout << "ERROR: lsViewer::Init - Could not create Pass Manager instance" << endl;
    }

    // Set Active
    setActive( true );

    return 0;
}

/*!
 * \brief lsViewer::Update
 *
 * Call our lsSceneManagers Update() function.
 * Then passes Update() along to the Hierarchy.
 *
 * \param render
 * \param delta
 */
void lsViewer::Update( double delta )
{
#ifdef VORTX_DEVUG_CONTROL
    cout << "##########################" << endl;
    cout << "# lsViewer::Update : " << getName() << endl;
    cout << "##########################" << endl;
#endif

    if( scenemanager )
    {
        scenemanager->Update( getRender(), delta );
    }

    // Pass it on
    if( getControlManager() ) getControlManager()->Update( delta );
    if( getNext_control() ) getNext_control()->Update( delta );
}

/**
 Optimize and render our control
 */
void lsViewer::Render()
{
#ifdef VORTX_DEVUG_CONTROL
    cout << "##########################" << endl;
    cout << "# lsViewer::Render : " << getName() << endl;
    cout << "##########################" << endl;
#endif

    if( passmanager )
    {
        passmanager->Render();
    }

    // Pass it on
    if( getControlManager() ) getControlManager()->Render();
    if( getNext_control() ) getNext_control()->Render();
}

void lsViewer::Resize( int x, int y, int width, int height  )
{
    if( passmanager )
    {
        passmanager->Resize( x, y, width, height );
    }

    // Pass it on
    if( getControlManager() ) getControlManager()->Resize( x, y, width, height );
    if( getNext_control() ) getNext_control()->Resize( x, y, width, height );
}


