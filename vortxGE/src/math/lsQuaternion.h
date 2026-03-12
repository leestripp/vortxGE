/**
 * @file lsQuaternion.h
 * @brief Quaternion class for 3D rotation representation
 * 
 * Quaternions provide a mathematically elegant way to represent
 * 3D rotations that avoids gimbal lock and provides smooth
 * interpolation (SLERP) between orientations.
 * 
 * A quaternion is represented as: w + xi + yj + zk
 * where w is the scalar component and (x,y,z) is the vector component.
 * 
 * @section advantages Advantages over Euler Angles
 * - No gimbal lock
 * - Smooth interpolation between rotations (SLERP)
 * - More efficient for concatenating rotations
 * - Compact (4 floats vs 3)
 * 
 * @section usage Usage
 * @code
 * // Create from Euler angles (radians)
 * lsQuaternion q(roll, pitch, yaw);
 * 
 * // Create from axis-angle
 * lsQuaternion q2(axis, angleDegrees);
 * 
 * // Convert to matrix for OpenGL
 * lsMatrix4x4 mat;
 * q.toMatrix4x4(mat);
 * 
 * // Interpolate between rotations
 * lsQuaternion result;
 * result.Slerp(q1, q2, 0.5f);  // Halfway between
 * @endcode
 * 
 * @see lsMatrix4x4 for conversion to matrix form
 * @see lsVector3 for axis representation
 */

#pragma once

#include "lsMatrix4x4.h"

// Error tolerance for floating-point comparisons
#define DELTA 1e-10

// Forward declaration
class lsVector3;

/**
 * @class lsQuaternion
 * @brief Quaternion for 3D rotation representation
 * 
 * Quaternions extend complex numbers to 4 dimensions,
 * providing a compact and efficient representation
 * for 3D rotations.
 * 
 * @section components Components
 * - w: Scalar component (cos(θ/2) for rotation θ)
 * - x, y, z: Vector components (axis * sin(θ/2))
 * 
 * @note For a unit quaternion: w² + x² + y² + z² = 1
 */
class lsQuaternion
{
public:
    /**
     * @brief Default constructor
     * 
     * Creates identity quaternion (no rotation):
     * w=1, x=0, y=0, z=0
     */
	lsQuaternion();
	
    /**
     * @brief Construct quaternion from components
     * @param w Scalar component
     * @param x X vector component
     * @param y Y vector component
     * @param z Z vector component
     */
	lsQuaternion(float w, float x, float y, float z);
	
    /**
     * @brief Construct from Euler angles (roll, pitch, yaw)
     * 
     * Converts Euler angles in radians to quaternion.
     * Rotation order is ZYX (yaw, pitch, roll).
     * 
     * @param roll Rotation around Z axis (radians)
     * @param pitch Rotation around X axis (radians)
     * @param yaw Rotation around Y axis (radians)
     */
	lsQuaternion(float roll, float pitch, float yaw);
	
    /**
     * @brief Construct from axis-angle representation
     * 
     * @param axis Rotation axis (should be normalized)
     * @param angle Rotation angle in degrees
     */
    lsQuaternion(const lsVector3& axis, float angle);
	
    /**
     * @brief Virtual destructor
     */
	virtual ~lsQuaternion();

	//=========================================================================
	// Operators
	//=========================================================================
	
    /**
     * @brief Multiply quaternions (composition)
     * 
     * Combines two rotations. The result represents
     * applying 'q2' then 'this' rotation.
     * 
     * @param q2 Second quaternion
     * @return Reference to this (modified)
     */
	lsQuaternion& operator*=( const lsQuaternion& q2 );
	
    /**
     * @brief Multiply quaternions (non-modifying)
     * @param q2 Second quaternion
     * @return New quaternion representing combined rotation
     */
	lsQuaternion operator*( const lsQuaternion& q2 );

	//=========================================================================
	// Manipulation Methods
	//=========================================================================
	
    /**
     * @brief Set all components
     * @param w Scalar component
     * @param x X component
     * @param y Y component
     * @param z Z component
     */
	void Set(float w, float x, float y, float z);
	
    /**
     * @brief Spherical linear interpolation
     * 
     * Interpolates between this quaternion and 'to'
     * using spherical linear interpolation (SLERP).
     * Provides smooth rotation transition.
     * 
     * @param to Target quaternion
     * @param t Interpolation factor [0,1]
     *         0 = this, 1 = to, 0.5 = halfway
     */
	void Slerp(const lsQuaternion &to, float t);
	
    /**
     * @brief SLERP from explicit start
     * 
     * @param from Starting quaternion
     * @param to Target quaternion  
     * @param t Interpolation factor [0,1]
     */
	void Slerp(const lsQuaternion &from, const lsQuaternion &to, float t);
	
    /**
     * @brief Linear interpolation
     * 
     * Simpler but lower quality interpolation than Slerp.
     * Does not maintain constant angular velocity.
     * 
     * @param to Target quaternion
     * @param t Interpolation factor [0,1]
     */
	void Lerp(const lsQuaternion &to, float t);

    //=========================================================================
	// Mathematical Operations
	//=========================================================================
	
    /**
     * @brief Calculate magnitude (length)
     * @return sqrt(w² + x² + y² + z²)
     */
    float magnitude() const;
    
    /**
     * @brief Normalize to unit length
     * 
     * Makes the quaternion have magnitude 1.
     * Required for valid rotation quaternions.
     */
    void normalize();
    
    /**
     * @brief Get conjugate
     * 
     * Negates the vector component: (w, -x, -y, -z)
     * For unit quaternions, conjugate = inverse.
     * 
     * @return Conjugate quaternion
     */
    lsQuaternion conjugate() const;
    
    /**
     * @brief Get inverse
     * 
     * The inverse rotates in the opposite direction.
     * For unit quaternions: inverse = conjugate.
     * 
     * @return Inverse quaternion
     */
    lsQuaternion inverse() const;
    
    /**
     * @brief Set from axis-angle representation
     * 
     * @param axis Rotation axis (will be normalized)
     * @param angle Rotation angle in degrees
     */
    void fromAxisAngle(const lsVector3& axis, float angle);
    
    /**
     * @brief Calculate dot product
     * 
     * Used for SLERP to determine shortest path.
     * 
     * @param other Other quaternion
     * @return w1*w2 + x1*x2 + y1*y2 + z1*z2
     */
    float dot(const lsQuaternion& other) const;
    
    /**
     * @brief Convert to 4x4 matrix
     * 
     * Converts quaternion rotation to a 4x4 matrix
     * suitable for OpenGL transformations.
     * 
     * @param mat Output matrix (must be pre-allocated)
     */
    void toMatrix4x4(lsMatrix4x4& mat) const;

	//=========================================================================
    // Public Data Members
    //=========================================================================
	
    /** @brief Scalar component */
	float w;
    
    /** @brief X vector component */
	float x;
    
    /** @brief Y vector component */
	float y;
    
    /** @brief Z vector component */
	float z;
};
