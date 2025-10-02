#include <iostream>
// debug
#include "../vortx_debug.h"
// core
#include "../vortxGE.h"

// render
#include "../render/lsRender.h"

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
lsControl::lsControl( lsRender *render, lsControl* p )
{
    control_type = LSCT_NONE;
    parent = p;

    // store render
    m_render = render;

    // Control hierarchy
    controlmanager = new lsControlManager( render );
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
 * \param render
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
 * \param render
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

lsRender *lsControl::getOgl() const
{
    return m_render;
}

void lsControl::setOgl(lsRender *value)
{
    m_render = value;
}

