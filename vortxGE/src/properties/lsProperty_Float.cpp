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
    if( str != "" )
    {
        // Accept both bare ("7.25") and toString ("( 7.25 )") formats:
        // strip tuple decoration before parsing.
        string cleaned = str;
        for (auto& ch : cleaned)
            if (ch == '(' || ch == ')' || ch == ',') ch = ' ';

        stringstream ss( cleaned );
        float rx;
        string leftover;
        if( (ss >> rx) && !(ss >> leftover) )
            Set_X( rx );
    }
    return toString();
}
