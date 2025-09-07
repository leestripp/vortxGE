
#include "lsLight_Spot.h"

lsLight_Spot::lsLight_Spot(lsNode* parent) : lsLight(parent)
{
    setLight_type(LSLT_SPOT);
    setName("Spot Light");

    // Default spot light has a 45 degree cone and a 1:1 aspect ratio
    Set_Perspective(45.0, 1.0, 1.0, 100.0);
    spot_cutoff = 20.0f;
    spot_outer_cutoff = 25.0f;
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
