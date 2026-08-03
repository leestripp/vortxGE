#include <sstream>

// math
#include "../math/lsVector3.h"

// datablocks
#include "../datablocks/lsString.h"

#include "lsProperty_Vector3.h"


lsProperty_Vector3::lsProperty_Vector3( lsVector3 *val ) : lsProperty()
{
	setProp_type( LSPROP_VECTOR3 );
	setProp_name( "Not set" );

	vec3 = val;
}

lsProperty_Vector3::~lsProperty_Vector3()
{
}

void lsProperty_Vector3::Set(float x, float y, float z)
{
	/* debug
	cout << "lsProperty_Vector3::Set : x,y,z ";
	cout << " ( " << x;
	cout << " , " << y;
	cout << " , " << z;
	cout << " ) " << endl;
	*/

	vec3->x = x;
	vec3->y = y;
	vec3->z = z;
}


const string& lsProperty_Vector3::toString()
{
	stringstream ss;

	ss << "( ";
	ss << vec3->x;
	ss << " , ";
	ss << vec3->y;
	ss << " , ";
	ss << vec3->z;
	ss << " )";
	output = ss.str();

	return output;
}

const string& lsProperty_Vector3::fromString( const string& str )
{
    if( str != "" )
    {
        // Accept both bare ("x y z") and toString ("( x , y , z )") formats.
        string cleaned = str;
        for (auto& ch : cleaned)
            if (ch == '(' || ch == ')' || ch == ',') ch = ' ';

        stringstream ss( cleaned );
        float rx, ry, rz;
        string leftover;
        if( (ss >> rx >> ry >> rz) && !(ss >> leftover) )
            Set( rx, ry, rz );
    }
    return toString();
}

float lsProperty_Vector3::Get_X()
{
	if( vec3 )
	{
		return vec3->x;
	}
	return 0.0;
}

float lsProperty_Vector3::Get_Y()
{
	if( vec3 )
	{
		return vec3->y;
	}
	return 0.0;
}


float lsProperty_Vector3::Get_Z()
{
	if( vec3 )
	{
		return vec3->z;
	}
	return 0.0;
}



void lsProperty_Vector3::Set_X(float val)
{
	if( vec3 )
	{
		vec3->x = val;
	}
}

void lsProperty_Vector3::Set_Y(float val)
{
	if( vec3 )
	{
		vec3->y = val;
	}
}

void lsProperty_Vector3::Set_Z(float val)
{
	if( vec3 )
	{
		vec3->z = val;
	}
}
