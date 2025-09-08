// datablocks
#include "../datablocks/lsString.h"

#include "lsProperty_Float.h"


lsProperty_Float::lsProperty_Float( float *val ) : lsProperty()
{
	setProp_type( LSPROP_FLOAT );
	setProp_name( "Not set" );

	vec1 = val;
}

lsProperty_Float::~lsProperty_Float()
{
}

void lsProperty_Float::Set(float x)
{
	vec1[0] = x;
}

float lsProperty_Float::Get_X()
{
	return vec1[0];
}

void lsProperty_Float::Set_X(float val)
{
	vec1[0] = val;
}


const string& lsProperty_Float::toString()
{
	stringstream ss;

	ss << "( ";
	ss << vec1[0];
	ss << " )";
	output = ss.str();

	return output;
}

const string& lsProperty_Float::fromString( const string& str )
{
    vector<string> list;
    lsString convert;
    float rx;

    if( str != "" )
    {
        list.clear();
        // split line into parts
        convert.SplitString( str, ' ', list );

        if( list.size() == 1 )
        {
            rx = convert.toFloat( list[0] );

            Set_X(rx);
        }
    }
    return toString();
}
