#include <cmath>
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
        buildShadowmap_matrix(render);
    }
}

void lsLight::buildShadowmap_matrix( lsRender* render )
{
    // Classic GL-style bias: NDC [-1,1] -> texture [0,1].
    lsFloat bias[] = {0.5, 0.0, 0.0, 0.0,
                      0.0, 0.5, 0.0, 0.0,
                      0.0, 0.0, 0.5, 0.0,
                      0.5, 0.5, 0.5, 1.0 };
    // FIX 2026-08-11 (Vulkan dark floor, 4/4 — see Plan.md "D1").
    // Vulkan renders offscreen FBOs through a negative-height viewport
    // (GL-convention Y-flip), so the shadow map texels are V-mirrored
    // relative to GL-convention sampling — same reason the blit stage
    // needs flipY. Compensate in the bias (y' = 0.5 - 0.5*y_ndc) so the
    // shared light shaders sample correctly on both backends.
    if( render && render->getType() == RenderType::Vulkan )
        bias[5] = -0.5;
    lsMatrix4x4 biasMatrix;
    biasMatrix = bias;
	
    // The Light's Projection and View matrices combined
    lsMatrix4x4 lightProjectionViewMatrix = frustum.getProjectionMatrix() * ViewMatrix;

    // L1.5: texel snapping for directional (orthographic) lights.
    // Round the light-space view origin to the shadow-map texel grid
    // so sub-texel light/camera movement does not shift the map —
    // kills shadow swimming/shimmering. Perspective lights (spots)
    // are not snapped (no fixed texel grid).
    if( getCamera_mode() == LSCM_ORTHOGRAPHIC &&
        getPixel_x() > 0 && getPixel_y() > 0 )
    {
        const float texelX = ( orth_right - orth_left ) / getPixel_x();
        const float texelY = ( orth_top - orth_bottom ) / getPixel_y();
        // world origin in light view space
        const lsVector3 origLS = ViewMatrix * lsVector3( 0.0f, 0.0f, 0.0f );
        const float ox = floorf( origLS.x / texelX + 0.5f ) * texelX - origLS.x;
        const float oy = floorf( origLS.y / texelY + 0.5f ) * texelY - origLS.y;
        lsFloat snap[] = { 1.0f, 0.0f, 0.0f, 0.0f,
                           0.0f, 1.0f, 0.0f, 0.0f,
                           0.0f, 0.0f, 1.0f, 0.0f,
                           ox,    oy,    0.0f, 1.0f };
        lsMatrix4x4 snapMatrix;
        snapMatrix = snap;
        lightProjectionViewMatrix = snapMatrix * lightProjectionViewMatrix;
    }

    // The final matrix to transform from world space to shadow map texture space
    ShadowmapMatrix = biasMatrix * lightProjectionViewMatrix;
}

// Base class, not used
const string& lsLight::toString()
{
    return save_str;
}

