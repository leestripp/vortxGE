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
#include "../math/lsVector4.h"

// datablocks
#include "../datablocks/lsString.h"

#include "lsProperty_Vector4.h"


lsProperty_Vector4::lsProperty_Vector4( lsVector4 *val ) : lsProperty()
{
	setProp_type( LSPROP_VECTOR4 );
	setProp_name( "Not set" );

	vec4 = val;
}

lsProperty_Vector4::~lsProperty_Vector4()
{
}

void lsProperty_Vector4::Set(float x, float y, float z, float w)
{
	vec4->x = x;
	vec4->y = y;
	vec4->z = z;
	vec4->w = w;

}

const string& lsProperty_Vector4::toString()
{
	stringstream ss;

	ss << "( ";
	ss << vec4->x;
	ss << " , ";
	ss << vec4->y;
	ss << " , ";
	ss << vec4->z;
	ss << " , ";
	ss << vec4->w;
	ss << " )";
	output = ss.str();

	return output;
}

const string& lsProperty_Vector4::fromString( const string& str )
{
    vector<string> list;
    lsString convert;
    float rx, ry, rz, rw;

    if( str != "" )
    {
        list.clear();
        // split line into parts
        convert.SplitString( str, ' ', list );

        if( list.size() == 4 )
        {
            rx = convert.toFloat( list[0] );
            ry = convert.toFloat( list[1] );
            rz = convert.toFloat( list[2] );
            rw = convert.toFloat( list[3] );

            Set(rx, ry, rz, rw);
        }
    }
    return toString();
}



float lsProperty_Vector4::Get_X()
{
	if( vec4 )
	{
		return vec4->x;
	}
	return 0.0;
}

float lsProperty_Vector4::Get_Y()
{
	if( vec4 )
	{
		return vec4->y;
	}
	return 0.0;
}


float lsProperty_Vector4::Get_Z()
{
	if( vec4 )
	{
		return vec4->z;
	}
	return 0.0;
}

float lsProperty_Vector4::Get_W()
{
	if( vec4 )
	{
		return vec4->w;
	}
	return 0.0;
}



void lsProperty_Vector4::Set_X(float val)
{
	if( vec4 )
	{
		vec4->x = val;
	}
}

void lsProperty_Vector4::Set_Y(float val)
{
	if( vec4 )
	{
		vec4->y = val;
	}
}

void lsProperty_Vector4::Set_Z(float val)
{
	if( vec4 )
	{
		vec4->z = val;
	}
}

void lsProperty_Vector4::Set_W(float val)
{
	if( vec4 )
	{
		vec4->w = val;
	}
}

