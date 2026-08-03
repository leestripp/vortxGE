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
void lsRandom::SetSeed( unsigned int seed )
{
	srand( seed );
}

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

