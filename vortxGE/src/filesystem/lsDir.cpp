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

#include "lsDir.h"

lsDir::lsDir()
{
}

lsDir::~lsDir()
{
}

string lsDir::getexepath()
{
  char proc[1024];
  string fullPath = "";
  string path = "";

  // build proc path
  pid_t pid = getpid();
  char buf[20] = {0};
  sprintf( buf, "%d", pid );
  string _link = "/proc/";
  _link.append( buf );
  _link.append( "/exe" );

  int ch = readlink( _link.c_str(), proc, 1024 );
  if( ch != -1 )
  {
    proc[ch] = 0;
    path = proc;
    string::size_type t = path.find_last_of("/");
    path = path.substr( 0, t );
  }
  fullPath = path + string("/");

  return fullPath;
}

