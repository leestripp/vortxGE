/**
 * @file lsTimer.h
 * @brief High-resolution timer for performance measurement
 * 
 * Provides high-resolution timing using POSIX gettimeofday.
 * Used for frame timing, profiling, and animation delta time.
 * 
 * @author vortxGE
 * @date 2024
 */

#ifndef LSTIMER_H_
#define LSTIMER_H_

#include <iostream>
#include <string>

#include <sys/time.h>
#include <unistd.h>


using namespace std;

/**
 * @class lsTimer
 * @brief High-resolution timer
 * 
 * Provides precise timing functions using system time:
 * - Start/stop timing
 * - Elapsed time measurement
 * - Sleep/delay functionality
 * 
 * Uses POSIX gettimeofday for microsecond precision.
 * 
 * @note Platform-specific: Uses Linux/POSIX time functions
 * 
 * @example
 * @code
 * lsTimer timer;
 * 
 * timer.start_Timer();
 * // ... do work ...
 * double elapsed = timer.get_ElapsedTime();
 * cout << "Work took " << elapsed << " seconds" << endl;
 * 
 * // Sleep for 16ms (~60 FPS)
 * timer.sleep(16.0);
 * @endcode
 */
class lsTimer
{
public:
	/// Default constructor
	lsTimer();
	
	/// Destructor
	virtual ~lsTimer();

    //=========================================================================
    // Timing
    //=========================================================================
	
    /**
     * @brief Start timer
     * 
     * Records current time as start point.
     */
	void start_Timer();
	
    /**
     * @brief Start timer with adjustment
     * 
     * @param rem Remaining time to account for
     * @param ajust Apply adjustment
     */
	void start_Timer( double rem, bool ajust );
	
    /**
     * @brief Get elapsed time
     * 
     * @return Time elapsed since start_Timer() was called (seconds)
     */
	double get_ElapsedTime();

    //=========================================================================
    // Sleep
    //=========================================================================
	
    /**
     * @brief Sleep for specified duration
     * 
     * @param msec Milliseconds to sleep
     */
	void sleep( double msec );

private:
	/// Last elapsed time (seconds)
	double elapsedTime;
	
	/// Start time
	timeval t1, t2;

};

#endif
