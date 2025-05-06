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
