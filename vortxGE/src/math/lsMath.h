/**
 * @file lsMath.h
 * @brief Commonly used math constants and functions for 3D space
 * 
 * Provides mathematical constants and utility functions used throughout
 * the engine for 3D graphics operations including vector math helpers.
 * 
 * @author vortxGE
 * @date 2024
 */

#pragma once

/// Mathematical constant PI (π)
#define lsPI 3.14159265358979323846

/// Degrees to radians conversion factor
#define ANG2RAD 3.14159265358979323846/180.0

/// Radians to degrees conversion factor
#define RAD2ANG 180.0/3.14159265358979323846

// array types
typedef float	Vector2f[2];	///< 2-component float vector (array)
typedef float	Vector3f[3];	///< 3-component float vector (array)
typedef float	Vector4f[4];	///< 4-component float vector (array)
typedef float	Matrix44f[16];	///< 4x4 float matrix (array, column-major)

// Forward declarations
class lsVector2;
class lsVector3;
class lsVector4;

/*!
 * @class lsMath
 * @brief Commonly used math functions for 3D space
 * 
 * Provides static-style math utility functions for common 3D graphics
 * operations such as vector products and approximations.
 * 
 * @note This class is typically used as a collection of static utility
 * methods rather than being instantiated.
 * 
 * @example
 * @code
 * lsMath math;
 * bool equal = math.approxEqual(a, b, 0.001f);
 * lsVector3 cross = math.Cross(v1, v2);
 * float dot = math.Dot(v1, v2);
 * @endcode
 */
class lsMath
{
public:
	/// Default constructor
	lsMath();
	
	/// Destructor
	virtual ~lsMath();
	
	/**
	 * @brief Check if two floats are approximately equal
	 * 
	 * Compares two floating-point values within a specified epsilon tolerance.
	 * This is useful for floating-point comparisons where exact equality
	 * may not be reliable.
	 * 
	 * @param a First float value to compare
	 * @param b Second float value to compare
	 * @param epsilon Maximum allowed difference between values (tolerance)
	 * @return true if |a - b| < epsilon, false otherwise
	 */
	bool approxEqual( float a, float b, float epsilon );
	
	/**
	 * @brief Calculate cross product of two 3D vectors
	 * 
	 * Computes the cross product (Vec1 × Vec2) which results in a vector
	 * perpendicular to both input vectors. The direction follows the
	 * right-hand rule.
	 * 
	 * @param Vec1 First input vector
	 * @param Vec2 Second input vector
	 * @return lsVector3 Resulting perpendicular vector
	 * @retval (0,0,0) if vectors are parallel or near-parallel
	 */
	lsVector3 Cross( const lsVector3& Vec1, const lsVector3& Vec2 );
	
	/**
	 * @brief Calculate dot product of two 3D vectors
	 * 
	 * Computes the dot product (Vec1 · Vec2) which equals the product of
	 * the magnitudes times the cosine of the angle between them.
	 * 
	 * @param Vec1 First input vector
	 * @param Vec2 Second input vector
	 * @return float Dot product value
	 * @retval > 0 when vectors are acute angle
	 * @retval 0 when vectors are perpendicular
	 * @retval < 0 when vectors are obtuse angle
	 */
	float Dot( const lsVector3& Vec1, const lsVector3& Vec2 );
};
