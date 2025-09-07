#include "../vortx_debug.h"

// Properties
#include "../properties/lsProperty.h"
#include "../properties/lsProperty_Vector4.h"

// animation
#include "../animation/lsAnimationManager.h"

// passes
#include "../passes/lsPass.h"

#include "lsLight.h"


lsLight::lsLight( lsNode* parent ) : lsCamera(parent)
{
    setNode_type( LSNT_LIGHT );
    setName( "Light" );
    light_type = LSLT_NONE;
    save_str = "";

    // Data
    ambient = lsVector4( 0, 0, 0, 1.0 );
    diffuse = lsVector4( 1.0, 1.0, 1.0, 1.0 );
    specular = lsVector4( 1.0, 1.0, 1.0, 1.0 );
    spot_cutoff = 180.0;
    spot_outer_cutoff = 180.0;

    // shadow map
    tex_shadowmap = 0;
    shadow_bias = 0.005;
    pixel_x = 1.0;
    pixel_y = 1.0;
    has_shadowmap = false;

    // Camera adds Positon,rotation,scale.
    // We nee materials
    lsProperty_Vector4 *pv4;

    pv4 = new lsProperty_Vector4( &ambient );
    pv4->setProp_name( "Ambient" );
    pv4->setProp_id( LSPROPID_AMBIENT );
    Add_Property( pv4 );

    pv4 = new lsProperty_Vector4( &diffuse );
    pv4->setProp_name( "Diffuse" );
    pv4->setProp_id( LSPROPID_DIFFUSE );
    Add_Property( pv4 );

    pv4 = new lsProperty_Vector4( &specular );
    pv4->setProp_name( "Specular" );
    pv4->setProp_id( LSPROPID_SPECULAR );
    Add_Property( pv4 );
}


lsLight::~lsLight()
{
}


void lsLight::Optimize( __attribute__((unused)) lsCamera* cam, lsPass* pass, __attribute__((unused)) lsScene* scn, __attribute__((unused)) lsOpenGL* opengl )
{
    // Build shadowman matrix
    switch( pass->getPass_type() )
    {
		case LSPT_DIRECTRENDER:
		case LSPT_FBO:
            if (has_shadowmap) {
			    buildShadowmap_matrix();
            }
			break;
        default:
            break;
    }
}

void lsLight::buildShadowmap_matrix()
{
    const GLfloat bias[] = {0.5, 0.0, 0.0, 0.0,
                            0.0, 0.5, 0.0, 0.0,
                            0.0, 0.0, 0.5, 0.0,
                            0.5, 0.5, 0.5, 1.0 };

    ShadowmapMatrix = bias;
    ShadowmapMatrix = ShadowmapMatrix * frustum.getProjectionMatrix();
    ShadowmapMatrix = ShadowmapMatrix * ViewMatrix;
}

// Base class, not used
const string& lsLight::toString()
{
    return save_str;
}

