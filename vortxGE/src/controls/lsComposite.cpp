#include <iostream>
// debug
#include "../vortx_debug.h"

// managers
#include "../assets/lsAssetManager.h"
#include "../scene/lsSceneManager.h"
#include "../passes/lsPassManager.h"

// Controls
#include "lsControlManager.h"

#include "lsComposite.h"

/*!
 * \brief lsComposite::lsComposite
 * \param p
 */
lsComposite::lsComposite( lsRender *render, lsControl* p ) : lsControl(render,p)
{
    setControl_type( LSCT_COMPOSITE );
    setName("Composite");
    setActive( true );

    // managers : call Init() to setup
    scenemanager = NULL;
    passmanager = NULL;
    assetmanager = NULL;

}

lsComposite::~lsComposite()
{
    if( passmanager ) delete passmanager;
    if( scenemanager ) delete scenemanager;
    if( assetmanager ) delete assetmanager;
}

/*!
 * \brief lsComposite::Init
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
int lsComposite::Init()
{
    // Asset Manager
    assetmanager = new lsAssetManager( getOgl() );
    if(! assetmanager )
    {
        // ERROR
        cout << "ERROR: lsComposite::Init - Could not create Asset Manager instance" << endl;
    }

    scenemanager = new lsSceneManager();
    if(! scenemanager )
    {
        // ERROR
        cout << "ERROR: lsComposite::Init - Could not create Scene Manager instance" << endl;
    }

    passmanager = new lsPassManager( getOgl() );
    if(! passmanager )
    {
        // ERROR
        cout << "ERROR: lsComposite::Init - Could not create Pass Manager instance" << endl;
    }

    // Make active
    setActive( true );

    return 0;
}


/**
 Update our control
 */
void lsComposite::Update( double delta )
{
#ifdef VORTX_DEBUG_NODES
    cout << "##########################" << endl;
    cout << "# lsComposite::Update    #" << endl;
    cout << "##########################" << endl;
#endif
    if( scenemanager )
    {
        scenemanager->Update( getOgl(), delta );
    }

    // Pass it on
    if( getControlManager() ) getControlManager()->Update( delta );
    if( getNext_control() ) getNext_control()->Update( delta );
}

/**
 Optimize and render our control
 */
void lsComposite::Render()
{
#ifdef VORTX_DEBUG_NODES
    cout << "##########################" << endl;
    cout << "# lsComposite::Render    #" << endl;
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

void lsComposite::Resize( int x, int y, int width, int height )
{
    if( passmanager )
    {
        passmanager->Resize( x, y, width, height );
    }

    // Pass it on
    if( getControlManager() ) getControlManager()->Resize( x, y, width, height );
    if( getNext_control() ) getNext_control()->Resize( x, y, width, height );
}


