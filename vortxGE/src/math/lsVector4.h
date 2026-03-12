/**
 * @file lsVector4.h
 * @brief 4D Vector class for homogeneous coordinates and colors
 * 
 * lsVector4 represents a 4D vector with x, y, z, w components.
 * Commonly used for:
 * - Homogeneous coordinates (w for perspective)
 * - Color components (RGBA)
 * - Quaternion representation
 * - Plane coefficients
 * 
 * @note When used for colors, values are typically in [0,1] range
 * @note When used for homogeneous coords, w=0 for directions, w=1 for points
 * 
 * @see lsVector2 for 2D vectors
 * @see lsVector3 for 3D vectors
 * @see lsQuaternion for rotation representation
 */

#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <cmath>

// array types
typedef float	Vector4f[4];

using namespace std;

/**
 * @class lsVector4
 * @brief Represents a 4D vector with x, y, z, w components
 * 
 * Used for:
 * - Homogeneous 3D coordinates (x/w, y/w, z/w)
 * - RGBA color values
 * - Mathematical operations requiring 4 components
 * 
 * @section color_usage Color Usage
 * When used for colors:
 * @code
 * lsVector4 red(1.0f, 0.0f, 0.0f, 1.0f);  // RGBA
 * lsVector4 transparent(0.0f, 0.0f, 0.0f, 0.0f);  // Fully transparent
 * @endcode
 */
class lsVector4
{
public:
    /**
     * @brief Default constructor
     * 
     * Creates a zero vector (0, 0, 0, 0)
     */
	lsVector4();
	
    /**
     * @brief Construct a vector with specified components
     * @param XX X component
     * @param YY Y component
     * @param ZZ Z component
     * @param WW W component (homogeneous weight or alpha)
     */
	lsVector4( float XX, float YY, float ZZ, float WW );
	
	//=========================================================================
	// Arithmetic Operators
	//=========================================================================
	
    /**
     * @brief Add scalar to all components
     * @param s Scalar value
     * @return New vector (x+s, y+s, z+s, w+s)
     */
	lsVector4 operator+(float s) const;
	
    /**
     * @brief Add two vectors component-wise
     * @param v Vector to add
     * @return New vector (x+v.x, y+v.y, z+v.z, w+v.w)
     */
	lsVector4 operator+(const lsVector4 &v) const;
	
    /**
     * @brief Subtract two vectors component-wise
     * @param v Vector to subtract
     * @return New vector (x-v.x, y-v.y, z-v.z, w-v.w)
     */
	lsVector4 operator-(const lsVector4 &v) const;
	
    /**
     * @brief Multiply vectors component-wise
     * @param v Vector to multiply
     * @return New vector (x*v.x, y*v.y, z*v.z, w*v.w)
     */
	lsVector4 operator*(const lsVector4 &v) const;
	
    /**
     * @brief Multiply by scalar
     * @param t Scalar multiplier
     * @return New vector (x*t, y*t, z*t, w*t)
     */
	lsVector4 operator*(float t) const;
	
    /**
     * @brief Divide by scalar
     * @param t Scalar divisor
     * @return New vector (x/t, y/t, z/t, w/t)
     */
	lsVector4 operator/(float t) const;
	
    /**
     * @brief Negate vector
     * @return New vector (-x, -y, -z, -w)
     */
	lsVector4 operator-() const;
	
	//=========================================================================
	// Vector Operations
	//=========================================================================
	
    /**
     * @brief Calculate length (magnitude)
     * @return sqrt(x² + y² + z² + w²)
     */
	float length();
	
    /**
     * @brief Normalize to unit length
     * 
     * Modifies this vector in-place to have length 1.
     */
	void normalize();

    /**
     * @brief Copy from another vector
     * @param v Source vector
     */
	void copy(const lsVector4 &v);
	
    /**
     * @brief Set all components
     * @param x X component
     * @param y Y component
     * @param z Z component
     * @param w W component
     */
	void set(float x,float y, float z, float w );
	
    /**
     * @brief Calculate distance to another point
     * @param V1 Other point
     * @return Distance between this and V1
     */
	float distance( const lsVector4 &V1 );

    //=========================================================================
	// Utility Methods
	//=========================================================================
	
    /**
     * @brief Get string representation
     * @return String in format "(x, y, z, w)"
     */
    const string& toString();
    
    /**
     * @brief Debug output
     * @param msg Message prefix
     */
    void debug(const string& msg);

	//=========================================================================
	// Inline Getters/Setters
	//=========================================================================
	
    /**
     * @brief Get X component
     * @return X value
     */
	float getX() const
	{
		return x;
	}

    /**
     * @brief Set X component
     * @param x New X value
     */
	void setX(float x)
	{
		this->x = x;
	}

    /**
     * @brief Get Y component
     * @return Y value
     */
	float getY() const
	{
		return y;
	}

    /**
     * @brief Set Y component
     * @param y New Y value
     */
	void setY(float y)
	{
		this->y = y;
	}

    /**
     * @brief Get Z component
     * @return Z value
     */
	float getZ() const
	{
		return z;
	}

    /**
     * @brief Set Z component
     * @param z New Z value
     */
	void setZ(float z)
	{
		this->z = z;
	}

    /**
     * @brief Get W component
     * @return W value
     */
	float getW() const
	{
		return w;
	}

    /**
     * @brief Set W component
     * @param w New W value
     */
	void setW(float w)
	{
		this->w = w;
	}
	
	//=========================================================================
    // Public Data Members
    //=========================================================================
	
    /** @brief X component */
	float x;
    
    /** @brief Y component */
	float y;
    
    /** @brief Z component */
	float z;
    
    /** @brief W component (homogeneous or alpha) */
	float w;
	
private:
    /** @brief Internal buffer for toString() */
    string output;
};
