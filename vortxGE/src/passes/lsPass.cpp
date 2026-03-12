/**
 * @file lsPass.cpp
 * @brief Rendering pass implementation
 * 
 * This file implements the base lsPass class which represents
 * a single rendering pass in the engine's render pipeline.
 * 
 * Passes can be chained together to create complex rendering effects.
 * Each pass can have its own:
 * - Scene to render
 * - Camera for view transformation
 * - Culling camera for visibility testing
 * - Light for shadow/lighting calculations
 * - Viewport dimensions
 * - Background color
 * - Custom properties
 * 
 * @see lsPass.h
 * @see lsPassManager.h
 * @see lsPass_DR.h
 * @see lsPass_FBO.h
 */

#include <iostream>

#include "../vortx_debug.h"

// openGL
#include "../render/lsRender.h"

// Properties
#include "../properties/lsProperty_Vector4.h"

// Scene
#include "../scene/lsScene.h"

#include "lsPass.h"


/**
 * @brief Construct a rendering pass
 * 
 * Initializes pass with default values:
 * - Type: LSPT_NONE
 * - Name: "Pass"
 * - Viewport: (0, 0) to (800, 600)
 * - Background: Red (1, 0, 0, 1)
 * 
 * @param render Pointer to renderer
 */

lsPass::lsPass( lsRender* render )
{
    pass_type = LSPT_NONE;
    name = "Pass";
    // stor render
    m_render = render;

    next_pass = NULL;
    prev_pass = NULL;

    scene = NULL;
    camera = NULL;
    cull_camera = NULL;
    light = NULL;

    // view defaults
	position = lsVector2( 0, 0 );
    size = lsVector2( 800, 600 );
    vratio = 1.0;
    background_colour = lsVector4( 1.0, 0, 0, 1.0 );

    // Properties
    property_list = NULL;

    lsProperty_Vector4 *pv4;

    pv4 = new lsProperty_Vector4( &background_colour );
    pv4->setProp_name( "Background Colour" );
    pv4->setProp_id( LSPROPID_DIFFUSE );
    Add_Property( pv4 );

}

lsPass::~lsPass()
{
    // deleted by Pass Manager

    // Self cleaning
    if( property_list ) delete property_list;

}

/**
 * @brief Add a property to the pass
 * 
 * Properties allow runtime customization of pass behavior.
 * They are stored in a singly-linked list.
 * 
 * New properties are added to the front of the list:
 * [new] -> [old] -> [older] -> ...
 * 
 * Common uses:
 * - Post-processing parameters
 * - Shader uniforms
 * - Render settings
 * 
 * @param p Property to add
 */

void lsPass::Add_Property( lsProperty* p )
{
    p->setNext_prop( property_list );
    if( property_list ) property_list->setPrev_prop( p );
    property_list = p;
}

lsRender *lsPass::getRender() const
{
    return m_render;
}

void lsPass::setOgl(lsRender *value)
{
    m_render = value;
}

