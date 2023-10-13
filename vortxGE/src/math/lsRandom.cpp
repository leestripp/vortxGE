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

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cassert>

#include "lsRandom.h"


using namespace std;

lsRandom::lsRandom()
{
	// TODO: is this good enough?
	srand( (unsigned)time(0) );
	srand( rand() );
}

lsRandom::~lsRandom()
{
}

//! Range 0 <-> max
/*!
 * Returns a random int number between 0 and max.
 *
 * \param max
 * \return
 */
int lsRandom::Range( int max )
{
	return (rand()%max);
}

//! Random float value
/*!
 * float value is between 0.0 and 1.0
 * 
 * */
float lsRandom::Rand_Float()
{
	float r = (float)rand()/(float)RAND_MAX;

	return r;
}

//! Range a <-> b
/*!
 * Returns a random float value between a and b.
 *
 * \param a
 * \param b
 * \return
 */
float lsRandom::Rand_FloatRange( float a, float b )
{
    if( a > b ) return 0.0;
	float r = ((b-a)*((float)rand()/(float)RAND_MAX))+a;
	return r;
}

