#include <iostream>

#include "../vortx_debug.h"

// openGL
#include "../opengl/lsOpenGL.h"

// Properties
#include "../properties/lsProperty_Vector4.h"

// Scene
#include "../scene/lsScene.h"

#include "lsPass.h"

lsPass::lsPass( lsOpenGL* opengl )
{
    pass_type = LSPT_NONE;
    name = "Pass";
    // stor opengl
    ogl = opengl;

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

void lsPass::Add_Property( lsProperty* p )
{
    p->setNext_prop( property_list );
    if( property_list ) property_list->setPrev_prop( p );
    property_list = p;
}

lsOpenGL *lsPass::getOgl() const
{
    return ogl;
}

void lsPass::setOgl(lsOpenGL *value)
{
    ogl = value;
}

