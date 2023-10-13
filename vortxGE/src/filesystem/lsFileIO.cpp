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

#include "lsFileIO.h"


lsFileIO::lsFileIO()
{
	line.clear();
	filename = "";

}

lsFileIO::~lsFileIO()
{
	if( outfile.is_open() ) outfile.close();
	if( infile.is_open() ) infile.close();
}

bool lsFileIO::open( const string& file, int mode )
{
	filename = file;
	filemode = mode;
	switch( filemode )
	{
		case LSIO_BIN_APPEND:
			outfile.open( filename.c_str(), ofstream::app | ofstream::out | ofstream::binary );
			break;
		case LSIO_BIN_CLEAR:
			outfile.open( filename.c_str(), ofstream::trunc | ofstream::out | ofstream::binary );
			break;
		case LSIO_LINE_APPEND:
			outfile.open( filename.c_str(), ofstream::app | ofstream::out );
			break;
		case LSIO_LINE_CLEAR:
			outfile.open( filename.c_str(), ofstream::trunc | ofstream::out );
			break;

		case LSIO_LINE_READ:
			infile.open( filename.c_str(), ifstream::in );
			break;
		case LSIO_BIN_READ:
			infile.open( filename.c_str(), ifstream::in | ifstream::binary );
			break;

		default:
			cout << "ERROR: unknown file mode" << endl;
			break;
	}

	if(( infile.is_open() )||( outfile.is_open() ))
	{
 		return true;
	} else
	{
        cout << "ERROR: could not open file: " << filename << endl;
		return false;
	}
}

bool lsFileIO::readline()
{
	if(! infile.is_open() ) return false;

	if( infile.eof() ) return false;
	infile.getline( buffer, LSIO_LINE_BUFFERSIZE );
	line = buffer;
	return true;
}


bool lsFileIO::writeline( const string& line )
{
	if(! outfile.is_open() ) return false;
	outfile << line << "\n";
	return true;
}

bool lsFileIO::readbyte()
{
	if(! infile.is_open() ) return false;

	if( infile.eof() ) return false;
	infile.read( &ch, 1 );
	return true;
}

bool lsFileIO::readblock( char* buff, int size )
{
	if(! infile.is_open() ) return false;

	if( infile.eof() ) return false;
	infile.read( buff, size );
	bytesread = (int)infile.gcount();
	return true;
}



bool lsFileIO::writeblock( char* buff, int size )
{
	if(! outfile.is_open() ) return false;

	outfile.write( buff, size );
	return true;
}


