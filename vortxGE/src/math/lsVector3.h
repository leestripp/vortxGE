/**
 * @file lsVector3.h
 * @brief 3D Vector class for positions, directions, and math operations
 * 
 * lsVector3 is one of the most frequently used classes in the engine.
 * It represents a 3D vector with x, y, z components and provides
 * common vector operations including:
 * - Arithmetic operations (add, subtract, multiply, divide)
 * - Dot product and cross product
 * - Length and normalization
 * - Rotation around axes
 * - Distance calculations
 * 
 * @note This class uses float precision. For higher precision,
 *       consider lsVector4 which uses doubles internally.
 * 
 * @see lsVector2 for 2D vectors
 * @see lsVector4 for 4D vectors/homogeneous coordinates
 */

#pragma once

#include <iostream>
#include <cmath>

// array types
typedef float	Vector3f[3];


using namespace std;

/**
 * @class lsVector3
 * @brief Represents a 3D vector with x, y, z components
 * 
 * Used throughout the engine for:
 * - Positions in 3D space
 * - Direction vectors
 * - Movement and velocity
 * - Color components (RGB)
 * - Texture coordinates (for 3D textures)
 * 
 * @section operations Supported Operations
 * 
 * - Arithmetic: +, -, *, / (scalar and vector)
 * - Dot product: Dot()
 * - Cross product: Cross()
 * - Rotation: RotateAboutX/Y/Z/Axis()
 * - Length: length(), normalize()
 * - Distance: distance()
 * 
 * @note Inline getters/setters are provided for performance:
 *       getX(), getY(), getZ(), setX(), setY(), setZ()
 */
class lsVector3
{
public:
    /**
     * @brief Default constructor
     * 
     * Creates a zero vector (0, 0, 0)
     */
	lsVector3();
	
    /**
     * @brief Construct a vector with specified components
     * @param XX X component
     * @param YY Y component
     * @param ZZ Z component
     */
	lsVector3( float XX, float YY, float ZZ );
	
	//=========================================================================
	// Arithmetic Operators
	//=========================================================================
	
    /**
     * @brief Add scalar to all components
     * @param s Scalar value to add
     * @return New vector with (x+s, y+s, z+s)
     */
	lsVector3 operator+(float s) const;
	
    /**
     * @brief Add two vectors component-wise
     * @param v Vector to add
     * @return New vector with (x+v.x, y+v.y, z+v.z)
     */
	lsVector3 operator+(const lsVector3 &v) const;
	
    /**
     * @brief Subtract scalar from all components
     * @param s Scalar value to subtract
     * @return New vector with (x-s, y-s, z-s)
     */
	lsVector3 operator-(float s) const;
	
    /**
     * @brief Subtract two vectors component-wise
     * @param v Vector to subtract
     * @return New vector with (x-v.x, y-v.y, z-v.z)
     */
	lsVector3 operator-(const lsVector3 &v) const;
	
    /**
     * @brief Multiply vector by scalar
     * @param t Scalar multiplier
     * @return New vector with (x*t, y*t, z*t)
     */
	lsVector3 operator*(float t) const;
	
    /**
     * @brief Multiply two vectors component-wise (Hadamard product)
     * @param v Vector to multiply
     * @return New vector with (x*v.x, y*v.y, z*v.z)
     */
	lsVector3 operator*(const lsVector3 &v) const;
	
    /**
     * @brief Divide vector by scalar
     * @param t Scalar divisor (must be non-zero)
     * @return New vector with (x/t, y/t, z/t)
     * @warning Does not check for division by zero
     */
	lsVector3 operator/(float t) const;
	
    /**
     * @brief Negate vector (unary minus)
     * @return New vector with (-x, -y, -z)
     */
	lsVector3 operator-() const;
	
    /**
     * @brief Add vector in-place
     * @param v Vector to add
     * @return Reference to this vector
     */
	lsVector3& operator+=(const lsVector3 &v);
	
    /**
     * @brief Subtract vector in-place
     * @param v Vector to subtract
     * @return Reference to this vector
     */
	lsVector3& operator-=(const lsVector3 &v);
	
	//=========================================================================
	// Vector Operations
	//=========================================================================
	
    /**
     * @brief Calculate cross product with another vector
     * 
     * The cross product produces a vector perpendicular to both
     * input vectors, following the right-hand rule.
     * 
     * @code
     * lsVector3 up(0, 1, 0);
     * lsVector3 forward(1, 0, 0);
     * lsVector3 right = forward.Cross(up); // Should be (0, 0, -1)
     * @endcode
     * 
     * @param v Second vector for cross product
     * @return New vector perpendicular to both (this x v)
     * @see Dot() for dot product
     */
	lsVector3 Cross(const lsVector3 &v) const;
	
    /**
     * @brief Calculate dot product with another vector
     * 
     * The dot product is a scalar representing the magnitude
     * of the projection of one vector onto another.
     * 
     * @code
     * lsVector3 a(1, 0, 0);
     * lsVector3 b(1, 0, 0);
     * float dot = a.Dot(b); // Returns 1.0 (parallel)
     * 
     * lsVector3 c(0, 1, 0);
     * float dot2 = a.Dot(c); // Returns 0.0 (perpendicular)
     * @endcode
     * 
     * @param v Second vector for dot product
     * @return Scalar dot product (this . v)
     * @retval > 0 Vectors are acute angle apart
     * @retval = 0 Vectors are perpendicular
     * @retval < 0 Vectors are obtuse angle apart
     * @see Cross() for cross product
     */
	float Dot(const lsVector3 &v) const;
	
    /**
     * @brief Rotate vector around X axis
     * @param angle Rotation angle in radians
     * @return Reference to this vector (modified in-place)
     */
	lsVector3& RotateAboutX(float angle);
	
    /**
     * @brief Rotate vector around Y axis
     * @param angle Rotation angle in radians
     * @return Reference to this vector (modified in-place)
     */
	lsVector3& RotateAboutY(float angle);
	
    /**
     * @brief Rotate vector around Z axis
     * @param angle Rotation angle in radians
     * @return Reference to this vector (modified in-place)
     */
	lsVector3& RotateAboutZ(float angle);
	
    /**
     * @brief Rotate vector around arbitrary axis
     * 
     * Uses Rodrigues' rotation formula for rotation
     * around an arbitrary axis.
     * 
     * @param angle Rotation angle in radians
     * @param axis Unit vector specifying rotation axis
     * @return Reference to this vector (modified in-place)
     */
	lsVector3& RotateAboutAxis(float angle, const lsVector3& axis);

	//=========================================================================
	// Utility Methods
	//=========================================================================
	
    /**
     * @brief Set vector to match another vector's components
     * @param val Source vector to copy from
     */
	void set( const lsVector3& val );
	
    /**
     * @brief Calculate the length (magnitude) of the vector
     * 
     * Uses Euclidean norm: sqrt(x² + y² + z²)
     * 
     * @return Length of the vector
     * @see normalize() to get unit vector
     * @see lengthSquared() for faster comparison (no sqrt)
     */
	float length();
	
    /**
     * @brief Normalize the vector to unit length
     * 
     * Converts the vector to a unit vector (length = 1)
     * pointing in the same direction.
     * 
     * @return Reference to this vector (modified in-place)
     * @warning Does not handle zero-length vectors
     * @see length() to get current length
     */
	lsVector3& normalize();

    /**
     * @brief Copy components from another vector
     * @param v Source vector to copy from
     * @deprecated Use set() or assignment operator instead
     */
	void copy(const lsVector3 &v);
	
    /**
     * @brief Set individual component values
     * @param x X component
     * @param y Y component
     * @param z Z component
     */
	void set(float x,float y, float z);

    /**
     * @brief Add another vector to this one in-place
     * @param vec Vector to add
     * @deprecated Use operator+= instead
     */
	void add( const lsVector3& vec );
	
    /**
     * @brief Multiply by scalar and return new vector
     * @param a Scalar multiplier
     * @return New scaled vector
     * @deprecated Use operator* instead
     */
	lsVector3 scalarMult(float a);
	
    /**
     * @brief Calculate distance to another point
     * 
     * Treats both vectors as points in 3D space and
     * calculates the Euclidean distance between them.
     * 
     * @param V1 Other point
     * @return Distance between this and V1
     * @see length() for vector magnitude
     */
	float distance( const lsVector3 &V1 );

    /**
     * @brief Get string representation of vector
     * @return String in format "(x, y, z)"
     */
	const string& toString();
	
    /**
     * @brief Debug output to console
     * @param msg Optional message prefix
     */
	void debug(const string& msg);
	
	//=========================================================================
	// Inline Getters/Setters (for performance)
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

    //=========================================================================
    // Public Data Members
    //=========================================================================
    
    /** @brief X component (also accessible via getX()/setX()) */
	float x;
    
    /** @brief Y component (also accessible via getY()/setY()) */
	float y;
    
    /** @brief Z component (also accessible via getZ()/setZ()) */
	float z;

private:
    /** @brief Internal string buffer for toString() */
	string output;
};
