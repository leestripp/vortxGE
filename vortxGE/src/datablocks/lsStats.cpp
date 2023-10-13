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

#include <iostream>
#include <string>

#include "../vortx_debug.h"

#include "lsStats.h"


using namespace std;


lsStats::lsStats()
{
	frameCount = 0;
	framerate = 0;
}

lsStats::~lsStats()
{
}

void lsStats::DisplayStats()
{
#ifdef VORTX_DEBUG_STATS
	cout << endl;
	cout << "Framerate : " << framerate << " (fps)" << endl;
    cout << "***** Loads ***** ***** *****" << endl;
    cout << "Move     Load  : " << move_ticks << " ticks" << endl;
    cout << "Optimize Load  : " << optimize_ticks << " ticks" << endl;
    cout << "Draw     Load  : " << draw_ticks << " ticks" << endl;
    cout << "Physics  Load  : " << physics_ticks << " ticks" << endl;

    // Memory - used_mem is in bytes 1024^2
    double mb = double(used_mem) / (1024.0*1024.0);
    cout << "***** Memory ***** ***** *****" << endl;
    cout << "Assets (bytes) : " << used_mem;
    cout << " (" << mb;
    cout << "MB)" << endl;
#endif
}

void lsStats::Init()
{
    // data counter
    used_mem = 0;
	
	// framerate
	frameCount = 0;
	framerate = 0;
	
    update_time = 10000.0; // every 10 secs
    // Loads
    move_ticks = 0;
    optimize_ticks = 0;
    draw_ticks = 0;
    physics_ticks = 0;

    update_timer.start_Timer();
}

/*!Update
 * Called every frame draw.
 * 
 * */
void lsStats::Update()
{
    double ticks;

	frameCount += 1.0;
	
    // Check output timer
    ticks = update_timer.get_ElapsedTime();
    if( ticks > update_time )
    {
		// get rate
		framerate = frameCount / (update_time/1000.0);
		
		DisplayStats();
		
		// reset
		update_timer.start_Timer();
		frameCount = 0;
    }
}


void lsStats::Loop_Start()
{
    move_ticks = 0;
    optimize_ticks = 0;
    draw_ticks = 0;
    physics_ticks = 0;
}

void lsStats::Loop_Stop()
{
}

void lsStats::Move_Start()
{
    move_timer.start_Timer();
}

void lsStats::Move_Stop()
{
    move_ticks += move_timer.get_ElapsedTime();
}

void lsStats::Optimize_Start()
{
    optimize_timer.start_Timer();
}

void lsStats::Optimize_Stop()
{
    optimize_ticks += optimize_timer.get_ElapsedTime();
}

void lsStats::Draw_Start()
{
    draw_timer.start_Timer();
}

void lsStats::Draw_Stop()
{
    draw_ticks += draw_timer.get_ElapsedTime();
}

void lsStats::Physics_Start()
{
    physics_timer.start_Timer();
}

void lsStats::Physics_Stop()
{
    physics_ticks += physics_timer.get_ElapsedTime();
}




