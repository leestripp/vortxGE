/**
 * @file lsVector2.h
 * @brief 2D Vector class for positions, UV coordinates, and 2D math
 * 
 * lsVector2 represents a 2D vector with x, y components.
 * Commonly used for:
 * - Texture coordinates (UV mapping)
 * - 2D positions on screen
 * - 2D velocities and directions
 * - Color components (RG)
 * 
 * @see lsVector3 for 3D vectors
 * @see lsVector4 for 4D vectors/homogeneous coordinates
 */

#pragma once

#include <sstream>
#include <string>

// array types
typedef float	Vector2f[2];

using namespace std;

/**
 * @class lsVector2
 * @brief Represents a 2D vector with x, y components
 * 
 * Lightweight 2D vector class used for texture coordinates,
 * screen positions, and 2D math operations.
 */
class lsVector2
{
public:
    /**
     * @brief Default constructor
     * 
     * Creates a zero vector (0, 0)
     */
	lsVector2();
	
    /**
     * @brief Construct a vector with specified components
     * @param v1 X component
     * @param v2 Y component
     */
	lsVector2( float v1, float v2 );
	
	//=========================================================================
	// Arithmetic Operators
	//=========================================================================
	
    /**
     * @brief Add two vectors component-wise
     * @param v Vector to add
     * @return New vector with (x+v.x, y+v.y)
     */
	lsVector2 operator+(const lsVector2 &v) const;
	
    /**
     * @brief Subtract two vectors component-wise
     * @param v Vector to subtract
     * @return New vector with (x-v.x, y-v.y)
     */
	lsVector2 operator-(const lsVector2 &v) const;
	
    /**
     * @brief Multiply two vectors component-wise (Hadamard product)
     * @param v Vector to multiply
     * @return New vector with (x*v.x, y*v.y)
     */
	lsVector2 operator*(const lsVector2 &v) const;
	
    /**
     * @brief Multiply vector by scalar
     * @param t Scalar multiplier
     * @return New vector with (x*t, y*t)
     */
	lsVector2 operator*(float t) const;
	
    /**
     * @brief Divide vector by scalar
     * @param t Scalar divisor
     * @return New vector with (x/t, y/t)
     */
	lsVector2 operator/(float t) const;
	
    //=========================================================================
	// Utility Methods
	//=========================================================================
	
    /**
     * @brief Get string representation
     * @return String in format "(x, y)"
     */
    const string& toString();
	
    //=========================================================================
    // Public Data Members
    //=========================================================================
    
    /** @brief X component */
	float x;
    
    /** @brief Y component */
	float y;
	
private:
    /** @brief Internal buffer for toString() */
    string output;
};
