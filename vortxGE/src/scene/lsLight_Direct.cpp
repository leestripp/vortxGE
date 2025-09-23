#include "lsLight_Direct.h"

lsLight_Direct::lsLight_Direct(lsNode* parent) : lsLight(parent)
{
    setLight_type(LSLT_DIRECT);
    setName("Direct Light");
	
    // Set more sensible defaults.
	// void Set_Orthographic( float xMin, float xMax, float yMin, float yMax, float zMin, float zMax );
    // void Set_Orthographic( float cube );
    Set_Orthographic(10.0);
    
    // Direct lights have no attenuation, so set this low.
    intensity = 5.0f;
}

lsLight_Direct::~lsLight_Direct()
{
}

const string& lsLight_Direct::toString()
{
    save_str = "add_light direct ";
    if( getName() != "" )
    {
        save_str += "`";
        save_str += getName();
        save_str += "`";
    } else
    {
        save_str += "`Direct`";
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
