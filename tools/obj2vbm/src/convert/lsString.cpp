/*
 * Copyright (C) 2015 by Lee A. Stripp <leestripp@gmail.com>
 *
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contributor(s):
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#include "lsString.h"

lsString::lsString()
{

}

lsString::~lsString()
{
}

int lsString::SplitString( const string& input, char delimiter, vector<string>& results )
{
    stringstream ss;
    ss.str(input);
    string item;

    while( getline(ss, item, delimiter) )
    {
        results.push_back( item );
    }

    return results.size();
}

int lsString::toInt( const string& input )
{
    int result = 0;

    std::istringstream ss( input );
    ss >> result;

    return( result );
}


float lsString::toFloat( const string& input )
{
    float result = 0;

    stringstream ss;
    ss << input;
    ss >> result;

    return( result );
}

double lsString::toDouble( const string& input  )
{
    double result = 0;

    stringstream ss;
    ss << input;
    ss >> result;

    return( result );
}

string lsString::RemoveExtension(const string &input)
{
    // get location
    size_t lastindex = input.find_last_of( "." );
    string rawname = input.substr( 0, lastindex );

    return rawname;
}

string lsString::ExtractPath(const string &input)
{
    size_t lastindex = input.find_last_of( "/" );
    if( lastindex == string::npos )
    {
        return "";
    }
    string rawpath = input.substr( 0, lastindex );

    return rawpath;
}


