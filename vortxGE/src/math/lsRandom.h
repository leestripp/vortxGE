#ifndef LSRANDOM_H_
#define LSRANDOM_H_

//! Random number generater
/*!
 * TODO: Find the best way for random on Linux.
 *
 */
class lsRandom
{
public:
	lsRandom();
	virtual ~lsRandom();

	int Range( int max );

	float Rand_Float();
	float Rand_FloatRange( float a, float b );

};

#endif /* LSRANDOM_H_ */

