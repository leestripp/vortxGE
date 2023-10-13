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
