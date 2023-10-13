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

#include "../convert/lsString.h"

#include "lsDir.h"

lsDir::lsDir()
{
}

lsDir::~lsDir()
{
}

#if defined(__linux__)
string lsDir::getexepath()
{
	lsString convert;
	vector<string> list;
	string path;
	string exe_path;
    char slash;

	slash = '/';
	path = "/"; // linux root

	char result[ PATH_MAX ];
	ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
	exe_path =  string( result, (count > 0) ? count : 0 );

	if( exe_path != "" )
	{
        convert.SplitString( exe_path, slash, list );

		for( int i=0; i<(int)list.size()-1; i++ )
		{
			path += list[i] + slash;
		}
	} else
	{
		path = "";
	}
	return path;
}
#endif /* __linux__ */


#if defined(__APPLE__)
string lsDir::getexepath()
{
	lsString convert;
	vector<string> list;
	string path;
	string exe_path;
	string slash;
	slash = '/';
	path = "/"; // linux root

	char result[1024];
	uint32_t size = sizeof(result);
	if( _NSGetExecutablePath(result, &size) == 0 )
		exe_path = result;
	else
		exe_path = "";

	if( exe_path != "" )
	{
		convert.SplitString( exe_path, slash, list, false );

		for( int i=0; i<(int)list.size()-1; i++ )
		{
			path += list[i] + slash;
		}
	} else
	{
		path = "";
	}
	return path;
}
#endif /* __APPLE__ */

