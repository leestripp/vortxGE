/*
 * Copyright (c) 2016 by Lee A. Stripp <leestripp@gmail.com>
 *
 * This file is part of the vortxGE library.
 *
 * vortxGE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * vortxGE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with vortxGE.  If not, see <http://www.gnu.org/licenses/>
 */

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
    vector<string> list;
    lsString convert;
    float rx, ry, rz;

    if( str != "" )
    {
        list.clear();
        // split line into parts
        convert.SplitString( str, ' ', list );

        if( list.size() == 3 )
        {
            rx = convert.toFloat( list[0] );
            ry = convert.toFloat( list[1] );
            rz = convert.toFloat( list[2] );

            Set(rx, ry, rz);
        }
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
