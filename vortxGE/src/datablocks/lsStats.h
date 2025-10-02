#ifndef LSSTATS_H_
#define LSSTATS_H_

#include "../timers/lsTimer.h"

//! Live stats
/**
  Store and display live game information.
  \n
 Created in vortxGE class, linked in lsRender class for easy access.

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
