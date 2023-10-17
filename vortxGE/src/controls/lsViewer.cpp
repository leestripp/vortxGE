/*
 * Copyright (c) 2016 by Lee A. Stripp <leestripp@gmail.com>
 *
 * This file is part of the vortxGE library.
 *
 * vortxGE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * vortxGE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with vortxGE.  If not, see <http://www.gnu.org/licenses/>
 */

#include "../vortx_debug.h"

#include <iostream>

#include "../assets/lsAssetManager.h"
#include "../scene/lsSceneManager.h"
#include "../passes/lsPassManager.h"
#include "lsControlManager.h"

#include "lsViewer.h"

lsViewer::lsViewer( lsOpenGL *opengl, lsControl *p ) : lsControl(opengl,p)
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
 * \param opengl - link to the lsOpenGL instance created in vortxGE
 * \return - error code: not implemented yet.
 */
int lsViewer::Init()
{
    // Asset Manager
    assetmanager = new lsAssetManager( getOgl() );
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

    passmanager = new lsPassManager( getOgl() );
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
 * \param opengl
 * \param delta
 */
void lsViewer::Update( double delta )
{
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
void lsViewer::Render()
{
#ifdef VORTX_DEBUG_NODES
    cout << "##########################" << endl;
    cout << "# lsViewer::Render       #" << endl;
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


