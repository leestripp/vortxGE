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
#include "../math/lsVector2.h"

// datablocks
#include "../datablocks/lsString.h"

#include "lsProperty_Vector2.h"


lsProperty_Vector2::lsProperty_Vector2( lsVector2 *val ) : lsProperty()
{
	setProp_type( LSPROP_VECTOR2 );
	setProp_name( "Not set" );

	vec2 = val;
}

lsProperty_Vector2::~lsProperty_Vector2()
{
}

void lsProperty_Vector2::Set(float x, float y)
{
	vec2->x = x;
	vec2->y = y;
}

const string& lsProperty_Vector2::toString()
{
	stringstream ss;

	ss << "( ";
	ss << vec2->x;
	ss << " , ";
	ss << vec2->y;
	ss << " )";
	output = ss.str();

	return output;
}

const string& lsProperty_Vector2::fromString( const string& str )
{
    vector<string> list;
    lsString convert;
    float rx, ry;

    if( str != "" )
    {
        list.clear();
        // split line into parts
        convert.SplitString( str, ' ', list );

        if( list.size() == 2 )
        {
            rx = convert.toFloat( list[0] );
            ry = convert.toFloat( list[1] );

            Set(rx, ry);
        }
    }
    return toString();
}


float lsProperty_Vector2::Get_X()
{
	if( vec2 )
	{
		return vec2->x;
	}
	return 0.0;
}

float lsProperty_Vector2::Get_Y()
{
	if( vec2 )
	{
		return vec2->y;
	}
	return 0.0;
}

void lsProperty_Vector2::Set_X(float val)
{
	if( vec2 )
	{
		vec2->x = val;
	}
}

void lsProperty_Vector2::Set_Y(float val)
{
	if( vec2 )
	{
		vec2->y = val;
	}
}

