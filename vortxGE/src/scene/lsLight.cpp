#include "../vortx_debug.h"

// Properties
#include "../properties/lsProperty.h"
#include "../properties/lsProperty_Vector4.h"

// animation
#include "../animation/lsAnimationManager.h"

// passes
#include "../passes/lsPass.h"

#include "../render/lsRender.h"

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
    intensity = 500.0f;
    spot_cutoff = 25.0;
    spot_outer_cutoff = 30.0;
	
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


void lsLight::Optimize( __attribute__((unused)) lsCamera* cam, __attribute__((unused)) lsPass* pass, __attribute__((unused)) lsScene* scn, lsRender* render )
{
    // If this light casts shadows, its shadow matrix needs to be calculated.
    // This should happen after its transform has been updated in the Move pass,
    // and before any pass that might need to sample its shadow map.
    if (has_shadowmap) {
        if( render ) m_flipShadowY = ( render->getType() == RenderType::Vulkan );
        buildShadowmap_matrix();
    }
}

void lsLight::buildShadowmap_matrix()
{
    // GL: NDC y-up maps straight to texture v (bottom-left origin).
    // Vulkan (negative-height viewport): FBO depth image is V-flipped,
    // so v = 0.5 - 0.5*y (row 0 = clip y=+1). Same convention fix as
    // the blit stage flipY — see Plan.md L0.2.
    const lsFloat biasGL[] = {0.5, 0.0, 0.0, 0.0,
                              0.0, 0.5, 0.0, 0.0,
                              0.0, 0.0, 0.5, 0.0,
                              0.5, 0.5, 0.5, 1.0 };
    // VK depth texture holds raw z_ndc (GL projection writes [-1,1]
    // through the [0,1] viewport unchanged on RADV), so the sampled z
    // must compare in the SAME space: identity z, not 0.5z+0.5.
    const lsFloat biasVK[] = {0.5,  0.0, 0.0, 0.0,
                              0.0, -0.5, 0.0, 0.0,
                              0.0,  0.0, 1.0, 0.0,
                              0.5,  0.5, 0.0, 1.0 };

    lsMatrix4x4 biasMatrix;
    biasMatrix = m_flipShadowY ? biasVK : biasGL;
	
    // The Light's Projection and View matrices combined
    lsMatrix4x4 lightProjectionViewMatrix = frustum.getProjectionMatrix() * ViewMatrix;
	
    // The final matrix to transform from world space to shadow map texture space
    ShadowmapMatrix = biasMatrix * lightProjectionViewMatrix;
}

// Base class, not used
const string& lsLight::toString()
{
    return save_str;
}

