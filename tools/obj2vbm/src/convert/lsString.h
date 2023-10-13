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

#ifndef LSSTRING_H_
#define LSSTRING_H_

#include <iostream>
#include <sstream>
#include <string>
#include <vector>


using namespace std;

//! String conversion
/*!
 * Split strings, convert to int and float etc...
 *
 */
class lsString
{
public:
    lsString();
    virtual ~lsString();

    // Strings
    int SplitString( const string& input, char delimiter, vector<string>& results );

    // Numbers
    int toInt( const string& input );
    float toFloat( const string& input  );
    double toDouble( const string& input  );

    // File utils
    string RemoveExtension( const string& input );
    string ExtractPath( const string& input );

};

#endif /* LSSTRING_H_ */
