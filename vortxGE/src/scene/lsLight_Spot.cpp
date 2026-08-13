#include "lsLight_Spot.h"

bool lsFitSpotDepthRange( const lsMatrix4x4& lightView,
                          const std::vector<lsVector3>& worldPositions,
                          const std::vector<float>& radii,
                          float& outNear, float& outFar )
{
    if( worldPositions.empty() ) return false;

    lsMatrix4x4 view = lightView; // operator* is non-const
    float nearZ = 1.0e30f, farZ = -1.0e30f;
    for( size_t i = 0; i < worldPositions.size(); ++i )
    {
        const lsVector3 p = view * worldPositions[i];
        const float d = -p.z; // light looks down -Z in view space
        const float r = ( i < radii.size() ) ? radii[i] : 0.0f;
        if( d - r < nearZ ) nearZ = d - r;
        if( d + r > farZ  ) farZ  = d + r;
    }

    // sane clamps: near must stay positive, far strictly beyond near
    if( nearZ < 0.05f ) nearZ = 0.05f;
    if( farZ < nearZ + 0.1f ) farZ = nearZ + 0.1f;

    outNear = nearZ;
    outFar  = farZ;
    return true;
}

lsLight_Spot::lsLight_Spot(lsNode* parent) : lsLight(parent)
{
    setLight_type(LSLT_SPOT);
    setName("Spot Light");
	
    // Set more sensible defaults. The perspective FOV should match the outer cone angle.
    // The demo scene will override the perspective settings anyway.
    spot_cutoff = 25.0f;
    spot_outer_cutoff = 30.5f;
    Set_Perspective(spot_outer_cutoff * 2.0, 1.0, 0.1, 100.0);
}

lsLight_Spot::~lsLight_Spot()
{
}

void lsLight_Spot::syncShadowFrustum()
{
    // The outer cone is where the light's contribution reaches zero —
    // the shadow frustum must cover exactly that (square map, circular
    // cone inscribed). Cone angles are public and editable at runtime,
    // so re-sync every shadow build; near/far are preserved (the
    // shadow stage fits them to content, L3.1).
    frustum.Set_Perspective( spot_outer_cutoff * 2.0, 1.0,
                             frustum.getNearplane(), frustum.getFarplane() );
}

const string& lsLight_Spot::toString()
{
    save_str = "add_light spot ";
    if( getName() != "" )
    {
        save_str += "`";
        save_str += getName();
        save_str += "`";
    } else
    {
        save_str += "`Spot`";
    }

    if( getParent() )
    {
        save_str += " `";
        save_str += getParent()->getName();
        save_str += "`";
    } else
    {
        save_str += " `NONE`";
    }

    // Pos, Rot and scale
    save_str += " ";
    save_str += position.toString();
    save_str += " ";
    save_str += rotation.toString();
    save_str += " ";
    save_str += scale.toString();

    return save_str;
}
