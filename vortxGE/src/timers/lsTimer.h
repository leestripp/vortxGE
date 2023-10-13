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

#ifndef LSTIMER_H_
#define LSTIMER_H_

#include <iostream>
#include <string>

#include <sys/time.h>
#include <unistd.h>


using namespace std;

//! Hires Timer
/*!
 * Hires Timer
 *
 */
class lsTimer
{
public:
	lsTimer();
	virtual ~lsTimer();

	void start_Timer();
	void start_Timer( double rem, bool ajust );
	double get_ElapsedTime();

	void sleep( double msec );

private:
	double elapsedTime;
	timeval t1, t2;

};

#endif
