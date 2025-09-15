
#include "lsLight_Spot.h"

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
