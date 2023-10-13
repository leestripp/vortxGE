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

#ifndef LSSTATS_H_
#define LSSTATS_H_

#include "../timers/lsTimer.h"

//! Live stats
/**
  Store and display live game information.
  \n
 Created in vortxGE class, linked in lsOpenGL class for easy access.

*/
class lsStats
{
public:
    lsStats();
    virtual ~lsStats();

    void Init();
    void Update();
    void DisplayStats();
    // Loads
    void Loop_Start();
    void Loop_Stop();
    void Move_Start();
    void Move_Stop();
    void Optimize_Start();
    void Optimize_Stop();
    void Draw_Start();
    void Draw_Stop();
    void Physics_Start();
    void Physics_Stop();


    // Memory
	void addUsed_mem( unsigned int val )
	{
		used_mem += val;
	}
	
    void setUsed_mem( unsigned int val )
    {
        used_mem = val;
    }

    unsigned int getUsed_mem() const
    {
        return used_mem;
    }


private:
    lsTimer	update_timer;
    double update_time;
	// framerate
	double frameCount;
	double framerate;
	
    lsTimer	move_timer;
    lsTimer	optimize_timer;
    lsTimer	draw_timer;
    lsTimer	physics_timer;
    // store ticks
    double move_ticks;
    double optimize_ticks;
    double draw_ticks;
    double physics_ticks;
	
    // Memory usage
    unsigned int used_mem;
	
};

#endif /* LSSTATS_H_ */
