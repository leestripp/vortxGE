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

// Controls
#include "lsComposite.h"
#include "lsViewer.h"

#include "lsControlManager.h"

/*!
 * \brief lsControlManager::lsControlManager
 */
lsControlManager::lsControlManager( lsOpenGL *opengl )
{
    control_list = NULL;
    ogl = opengl;
}

lsControlManager::~lsControlManager()
{
    while( control_list )
    {
        Del_Control( control_list );
    }
}


// #########################################

void lsControlManager::Update( double delta )
{
    if( control_list ) control_list->Update( delta );
}

void lsControlManager::Render()
{
    if( control_list ) control_list->Render();
}

void lsControlManager::Resize( int x, int y, int width, int height  )
{
    if( control_list ) control_list->Resize( x, y, width, height );
}


// #########################################

void lsControlManager::Add_Control( lsControl* control )
{
#ifdef VORTX_DEVUG_CONTROL
    cout << "lsControlManager::Add_Control() - Name: " << control->getName() << endl;
#endif

    control->setNext_control( control_list );
    if( control_list ) control_list->setPrev_control( control );
    control_list = control;
}

void lsControlManager::Del_Control( lsControl* control )
{
#ifdef VORTX_DEVUG_CONTROL
    cout << "lsControlManager::Del_Control() - Name: " << control->getName() << endl;
#endif

    if( control->getPrev_control() == NULL )
    {
        // first in list
        control_list = control->getNext_control();
        if( control_list ) control_list->setPrev_control( NULL );

    } else
    {
        // Close the gap
        control->getPrev_control()->setNext_control( control->getNext_control() );
        if( control->getNext_control() )
        {
            control->getNext_control()->setPrev_control( control->getPrev_control() );
        }
    }
    delete control;
}

lsOpenGL *lsControlManager::getOgl() const
{
    return ogl;
}

void lsControlManager::setOgl(lsOpenGL *value)
{
    ogl = value;
}

