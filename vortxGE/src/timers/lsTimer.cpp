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

#include "lsTimer.h"

lsTimer::lsTimer()
{

}

lsTimer::~lsTimer()
{
}


void lsTimer::start_Timer()
{
	// start timer
	gettimeofday( &t1, NULL );

}

void lsTimer::start_Timer( double rem, bool ajust )
{
	int usec;
	// convert
	usec = rem*1000.0f;
	start_Timer();
	if( ajust )
	{
		// Debug
		// std::cout << "Timer usecs adjust - " << usec << std::endl;
		t1.tv_usec += usec;
	}
}


double lsTimer::get_ElapsedTime()
{
	// stop timer
	gettimeofday( &t2, NULL );

	// compute and return the elapsed time in millisec
	elapsedTime = ( t2.tv_sec - t1.tv_sec ) * 1000.0f;			// sec to ms
	elapsedTime += ( t2.tv_usec - t1.tv_usec ) / 1000.0f;			// us to ms

	return elapsedTime;
}

void lsTimer::sleep( double msec )
{
	unsigned int usec;

	// convert
	usec = msec*1000.0f;
	// Debug
	// std::cout << "Timer usecs sleep - " << usec << std::endl;

	usleep( usec );
}



