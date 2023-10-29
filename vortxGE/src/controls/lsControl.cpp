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

#include <iostream>
// debug
#include "../vortx_debug.h"
// core
#include "../vortxGE.h"

// opengl
#include "../opengl/lsOpenGL.h"

// Assets
#include "../assets/lsAssetManager.h"

// Controls
#include "lsControlManager.h"
#include "lsControl.h"

/*!
 * \brief Base class for all Controls
 *
 * Never use this class directly, create a subclass.\n
 * You MUST call Init() before using any subclass Control object.\n
 * \n
 *
 * \param p - link to parent control.
 */
lsControl::lsControl( lsOpenGL *opengl, lsControl* p )
{
    control_type = LSCT_NONE;
    parent = p;

    // store opengl
    ogl = opengl;

    // Control hierarchy
    controlmanager = new lsControlManager( opengl );
    if(! controlmanager )
    {
        // ERROR
        cout << "ERROR: lsControl::lsControl - Could not create Control Manager instance" << endl;
    }

    // default data
    name = "Control";
    active = false;

    // empty list
    next_control = NULL;
    prev_control = NULL;

}

/*!
 * \brief lsControl::~lsControl
 *
 * Deletes attached Control Manager.\n
 *
 */
lsControl::~lsControl()
{
    // clean up
    if( controlmanager ) delete controlmanager;
}


/*!
 * \brief lsControl::Update
 * \param opengl
 * \param delta
 */
void lsControl::Update( double delta )
{
    // Pass it on
    if( controlmanager ) controlmanager->Update( delta );
    if( next_control ) next_control->Update( delta );
}

/*!
 * \brief lsControl::Render
 * \param opengl
 */
void lsControl::Render()
{
    // Pass it on
    if( controlmanager ) controlmanager->Render();
    if( next_control ) next_control->Render();
}

/*!
 * \brief lsControl::Resize
 * \param width
 * \param height
 */
void lsControl::Resize( int x, int y, int width, int height  )
{
    // Pass it on
    if( controlmanager ) controlmanager->Resize( x, y, width, height );
    if( next_control ) next_control->Resize( x, y, width, height );
}



// ###############################
// Control heirarchy

void lsControl::Add_Control( lsControl* val )
{
    if( controlmanager )
    {
        controlmanager->Add_Control( val );
    }
}

lsOpenGL *lsControl::getOgl() const
{
    return ogl;
}

void lsControl::setOgl(lsOpenGL *value)
{
    ogl = value;
}

