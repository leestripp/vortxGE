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

#ifndef LSSTRING_H_
#define LSSTRING_H_

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>

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

    static int SplitString( const string& input, char delimiter, vector<string>& results );
	int toInt( const string& input );
	float toFloat( const string& input  );
	double toDouble( const string& input  );

    // File utils
    string RemoveExtension( const string& input );
    string ExtractPath( const string& input );

};

#endif /* LSSTRING_H_ */
