#include <iostream>
// debug
#include "../vortx_debug.h"

// Controls
#include "lsComposite.h"
#include "lsViewer.h"

#include "lsControlManager.h"

/*!
 * \brief lsControlManager::lsControlManager
 */
lsControlManager::lsControlManager( lsRender *render )
{
    control_list = NULL;
    m_render = render;
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
#ifdef VORTX_DEVUG_CONTROL
    cout << "lsControlManager::Update" << endl;
#endif

    if( control_list ) control_list->Update( delta );
}

void lsControlManager::Render()
{
#ifdef VORTX_DEVUG_CONTROL
    cout << "lsControlManager::Render" << endl;
#endif

    if( control_list ) control_list->Render();
}

void lsControlManager::Resize( int x, int y, int width, int height  )
{
#ifdef VORTX_DEVUG_CONTROL
    cout << "lsControlManager::Resize" << endl;
#endif

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

lsRender *lsControlManager::getRender() const
{
    return m_render;
}

void lsControlManager::setOgl(lsRender *value)
{
    m_render = value;
}

