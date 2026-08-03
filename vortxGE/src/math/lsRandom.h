/**
 * @file lsRandom.h
 * @brief Random number generation utilities
 * 
 * Provides random number generation for various use cases including
 * game logic, procedural generation, and simulations.
 * 
 * @author vortxGE
 * @date 2024
 * @todo Find the best way for random number generation on Linux
 */

#ifndef LSRANDOM_H_
#define LSRANDOM_H_

/**
 * @class lsRandom
 * @brief Random number generator
 * 
 * Provides methods for generating random numbers in various ranges
 * and formats. Useful for game mechanics, procedural content generation,
 * and simulations.
 * 
 * @note Currently uses C-style rand() - may want to replace with
 * Mersenne Twister or other higher-quality RNG for production use.
 * 
 * @example
 * @code
 * lsRandom rng;
 * 
 * // Random integer in range [0, max)
 * int dieRoll = rng.Range(6);
 * 
 * // Random float in range [0, 1)
 * float chance = rng.Rand_Float();
 * 
 * // Random float in range [a, b)
 * float position = rng.Rand_FloatRange(-10.0f, 10.0f);
 * @endcode
 */
class lsRandom
{
public:
	/// Default constructor - initializes random seed
	lsRandom();
	
	/// Set a fixed seed for reproducible sequences.
	/// NOTE: uses global rand() state — affects all lsRandom instances.
	void SetSeed( unsigned int seed );
	
	/// Destructor
	virtual ~lsRandom();

	/**
	 * @brief Generate random integer in range [0, max)
	 * 
	 * Returns a random integer from 0 up to (but not including) max.
	 * 
	 * @param max Upper bound (exclusive)
	 * @return int Random integer in range [0, max)
	 */
	int Range( int max );

	/**
	 * @brief Generate random float in range [0, 1)
	 * 
	 * Returns a uniformly distributed random floating-point number
	 * in the range [0.0, 1.0).
	 * 
	 * @return float Random float in [0, 1)
	 */
	float Rand_Float();

	/**
	 * @brief Generate random float in custom range
	 * 
	 * Returns a uniformly distributed random floating-point number
	 * in the range [a, b).
	 * 
	 * @param a Lower bound (inclusive)
	 * @param b Upper bound (exclusive)
	 * @return float Random float in [a, b)
	 */
	float Rand_FloatRange( float a, float b );

};

#endif /* LSRANDOM_H_ */
