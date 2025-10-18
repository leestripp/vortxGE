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
		// cout << "Timer usecs adjust - " << usec << endl;
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
	// cout << "Timer usecs sleep - " << usec << endl;

	usleep( usec );
}



