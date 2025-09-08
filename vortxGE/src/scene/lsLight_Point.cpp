#include "lsLight_Point.h"


lsLight_Point::lsLight_Point( lsNode *parent ) : lsLight(parent)
{
    setLight_type( LSLT_POINT );

    // Can I use 180 for this, run tests
    Set_Perspective( 180.0, 1.0, 1.0, 1000.0 );

}

lsLight_Point::~lsLight_Point()
{
}


// Save data
const string& lsLight_Point::toString()
{
    save_str = "add_light point ";
    if( getName() != "" )
    {
        save_str += "`";
        save_str += getName();
        save_str += "`";
    } else
    {
        save_str += "`Point`";
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

