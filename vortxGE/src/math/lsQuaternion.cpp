/**
 * @file lsQuaternion.cpp
 * @brief Quaternion implementation for 3D rotations
 * 
 * This file implements quaternion mathematics for representing
 * and interpolating 3D rotations. Quaternions avoid gimbal lock
 * and provide smooth interpolation compared to Euler angles.
 * 
 * Key features:
 * - Rotation representation (axis-angle, Euler angles)
 * - Spherical linear interpolation (SLERP)
 * - Linear interpolation (LERP)
 * - Conversion to rotation matrix
 * 
 * @note Quaternions must be normalized for rotation use
 * @note A unit quaternion q and -q represent the same rotation
 * 
 * @see lsQuaternion.h
 * @see lsMatrix4x4.h
 * @see lsVector3.h
 */

#include <cmath>

#include "lsQuaternion.h"
#include "lsVector3.h"
#include "lsMath.h"


//=========================================================================
// Constructors
//=========================================================================

/**
 * @brief Default constructor
 * 
 * Creates identity quaternion representing no rotation.
 * Identity: w=1, (x,y,z)=(0,0,0)
 */
lsQuaternion::lsQuaternion()
{
	w = 1.0;
	x = 0;
	y = 0;
	z = 0;
}

/**
 * @brief Construct quaternion from components
 * 
 * @param w Scalar component
 * @param x Vector component X
 * @param y Vector component Y
 * @param z Vector component Z
 */
lsQuaternion::lsQuaternion(float w, float x, float y, float z)
{
	this->w = w;
	this->x = x;
	this->y = y;
	this->z = z;
}

/**
 * @brief Construct quaternion from Euler angles (roll, pitch, yaw)
 * 
 * Converts Euler angles in ZYX order to quaternion.
 * This is the standard flight/rotation order.
 * 
 * The conversion uses half-angle formulas:
 * q = q_yaw * q_pitch * q_roll
 * 
 * @param roll Rotation around X axis (degrees)
 * @param pitch Rotation around Y axis (degrees)
 * @param yaw Rotation around Z axis (degrees)
 */
lsQuaternion::lsQuaternion(float roll, float pitch, float yaw)
{
	// Half angles for trigonometric identities
	float cr = cos(roll*0.5);
	float cp = cos(pitch*0.5);
	float cy = cos(yaw*0.5);

	float sr = sin(roll/2);
	float sp = sin(pitch/2);
	float sy = sin(yaw/2);

	// Compute intermediate products
	float cpcy = cp * cy;
	float spsy = sp * sy;
	float cpsy = cp * sy;
	float spcy = sp * cy;

	// Combine rotations (ZYX order)
	w = cr * cpcy + sr * spsy;
	x = sr * cpcy - cr * spsy;
	y = cr * spcy + sr * cpsy;
	z = cr * cpsy - sr * spcy;
}

/**
 * @brief Construct quaternion from axis-angle
 * 
 * @param axis Rotation axis (will be normalized)
 * @param angle Rotation angle in degrees
 */
lsQuaternion::lsQuaternion(const lsVector3& axis, float angle)
{
    fromAxisAngle(axis, angle);
}

/**
 * @brief Destructor
 */
lsQuaternion::~lsQuaternion()
{
}

//=========================================================================
// Quaternion Multiplication
//=========================================================================

/**
 * @brief Multiplication assignment operator (this *= q)
 * 
 * Quaternion multiplication combines rotations.
 * q1 * q2 means: first apply q2, then q1
 * (note: multiplication is NOT commutative)
 * 
 * Uses an optimized formula avoiding explicit
 * multiplications with the identity element.
 * 
 * @param q Quaternion to multiply
 * @return Reference to this
 */
lsQuaternion& lsQuaternion::operator*=(const lsQuaternion& q)
{
	float E, F, G, H;

	float tw = w;
	float tx = x;
	float ty = y;

	// Hamilton product optimized form
	E = (tx +  z)*(q.x + q.y);
	F = (tx -  z)*(q.x - q.y);
	G = (tw + ty)*(q.w - q.z);
	H = (tw - ty)*(q.w + q.z);

	w = ( z - ty)*(q.y - q.z) + (-E - F + G + H)/2;
	x = (tw + tx)*(q.w + q.x) - ( E + F + G + H)/2;
	y = (tw - tx)*(q.y + q.z) + ( E - F + G - H)/2;
	z = (ty +  z)*(q.w - q.x) + ( E - F - G + H)/2;

	return *this;
}

/**
 * @brief Quaternion multiplication (this * q2)
 * 
 * Returns combined rotation: first q2, then this.
 * 
 * @param q2 Right-hand quaternion
 * @return Result of multiplication
 */
lsQuaternion lsQuaternion::operator*(const lsQuaternion& q2)
{
	lsQuaternion qr;

	float E, F, G, H;

	// Hamilton product
	E = (x + z)*(q2.x + q2.y);
	F = (x - z)*(q2.x - q2.y);
	G = (w + y)*(q2.w - q2.z);
	H = (w - y)*(q2.w + q2.z);

	qr.w = (z - y)*(q2.y - q2.z) + (-E - F + G + H)*0.5;
	qr.x = (w + x)*(q2.w + q2.x) - ( E + F + G + H)*0.5;
	qr.y = (w - x)*(q2.y + q2.z) + ( E - F + G - H)*0.5;
	qr.z = (y + z)*(q2.w - q2.x) + ( E - F - G + H)*0.5;

	return qr;
}

//=========================================================================
// Assignment and Setters
//=========================================================================

/**
 * @brief Set quaternion components
 * 
 * @param w Scalar component
 * @param x Vector X component
 * @param y Vector Y component
 * @param z Vector Z component
 */
void lsQuaternion::Set(float w, float x, float y, float z)
{
	this->w = w;
	this->x = x;
	this->y = y;
	this->z = z;
}

//=========================================================================
// Interpolation
//=========================================================================

/**
 * @brief Spherical Linear Interpolation (SLERP)
 * 
 * Interpolates from this quaternion TO another quaternion.
 * SLERP provides smooth rotation interpolation along the
 * shortest arc on the unit quaternion sphere.
 * 
 * Formula:
 * slerp(q1, q2, t) = (q1*sin((1-t)*ω) + q2*sin(t*ω)) / sin(ω)
 * 
 * Where ω is the angle between quaternions.
 * 
 * @param to Target quaternion
 * @param t Interpolation parameter [0, 1]
 * 
 * @note t=0 returns this, t=1 returns 'to'
 * @note Automatically handles antipodal quaternions
 */
void lsQuaternion::Slerp(const lsQuaternion& to, float t)
{
	float to1[4];
	float omega, cosom, sinom;
	float scale0, scale1;

	// Calculate dot product (cosine of half-angle)
	cosom = x*to.x + y*to.y + z*to.z + w*to.w;

	// Adjust signs if necessary (use shorter path)
	if (cosom < 0.0)
	{
		cosom = -cosom;
		to1[0] = - to.x;
		to1[1] = - to.y;
		to1[2] = - to.z;
		to1[3] = - to.w;
	} else  {
		to1[0] = to.x;
		to1[1] = to.y;
		to1[2] = to.z;
		to1[3] = to.w;
	}

	// Calculate interpolation coefficients
	if ((1.0 - cosom) > DELTA)
	{
		// Standard case: quaternions are not too close
		omega = acos(cosom);
		sinom = sin(omega);
		scale0 = (float)sin((1.0 - t) * omega) / sinom;
		scale1 = (float)sin(t * omega) / sinom;
	} else {
		// Quaternions are very close - use linear interpolation
		scale0 = 1.0f - t;
		scale1 = t;
	}

	// Calculate final interpolated quaternion
	x = scale0*x + scale1*to1[0];
	y = scale0*y + scale1*to1[1];
	z = scale0*z + scale1*to1[2];
	w = scale0*w + scale1*to1[3];
}

/**
 * @brief SLERP from explicit start to end
 * 
 * Static version allowing specification of both start and end.
 * 
 * @param from Starting quaternion
 * @param to Ending quaternion
 * @param t Parameter [0, 1]
 */
void lsQuaternion::Slerp(const lsQuaternion& from, const lsQuaternion& to, float t)
{
	float to1[4];
	float omega, cosom, sinom;
	float scale0, scale1;

	// Calculate dot product
	cosom = from.x*to.x + from.y*to.y + from.z*to.z + from.w*to.w;

	// Adjust signs for shortest path
	if (cosom < 0.0)
	{
		cosom = -cosom;
		to1[0] = - to.x;
		to1[1] = - to.y;
		to1[2] = - to.z;
		to1[3] = - to.w;
	} else
	{
		to1[0] = to.x;
		to1[1] = to.y;
		to1[2] = to.z;
		to1[3] = to.w;
	}

	// Calculate coefficients
	if ((1.0 - cosom) > DELTA)
	{
		// Standard case
		omega = acos(cosom);
		sinom = sin(omega);
		scale0 = (float)sin((1.0 - t) * omega) / sinom;
		scale1 = (float)sin(t * omega) / sinom;
	} else
	{
		// Close quaternions - linear interpolation
		scale0 = 1.0f - t;
		scale1 = t;
	}

	// Interpolate
	x = scale0*from.x + scale1*to1[0];
	y = scale0*from.y + scale1*to1[1];
	z = scale0*from.z + scale1*to1[2];
	w = scale0*from.w + scale1*to1[3];
}

/**
 * @brief Linear Interpolation (LERP)
 * 
 * Simpler but lower quality than SLERP. Interpolates
 * in a straight line through quaternion space rather
 * along the spherical arc.
 * 
 * Faster than SLERP but can result in non-constant
 * angular velocity and is not normalized during interpolation.
 * 
 * @param to Target quaternion
 * @param t Parameter [0, 1]
 * 
 * @note Result should be normalized after interpolation
 */
void lsQuaternion::Lerp(const lsQuaternion& to, float t)
{
	float to1[4];
	float cosom;
	float scale0, scale1;

	// Calculate dot product
	cosom = x*to.x + y*to.y + z*to.z + w*to.w;

	// Adjust signs for shortest path
	if( cosom < 0.0 )
	{
		to1[0] = - to.x;
		to1[1] = - to.y;
		to1[2] = - to.z;
		to1[3] = - to.w;
	} else
	{
		to1[0] = to.x;
		to1[1] = to.y;
		to1[2] = to.z;
		to1[3] = to.w;
	}

	// Linear interpolation
	scale0 = 1.0f - t;
	scale1 = t;

	// Calculate final values
	x = scale0*x + scale1*to1[0];
	y = scale0*y + scale1*to1[1];
	z = scale0*z + scale1*to1[2];
	w = scale0*w + scale1*to1[3];
}

//=========================================================================
// Norm and Normalization
//=========================================================================

/**
 * @brief Calculate quaternion magnitude
 * 
 * For unit quaternions (rotation), magnitude should be 1.
 * 
 * @return sqrt(w² + x² + y² + z²)
 */
float lsQuaternion::magnitude() const
{
    return sqrt(w * w + x * x + y * y + z * z);
}

/**
 * @brief Normalize quaternion to unit length
 * 
 * Essential for using quaternion as a rotation.
 * Divides all components by the magnitude.
 */
void lsQuaternion::normalize()
{
    float mag = magnitude();
    if (mag > 0.0f)
    {
        float oneOverMag = 1.0f / mag;
        w *= oneOverMag;
        x *= oneOverMag;
        y *= oneOverMag;
        z *= oneOverMag;
    }
}

//=========================================================================
// Inverse Operations
//=========================================================================

/**
 * @brief Get conjugate quaternion
 * 
 * The conjugate of a quaternion q = (w, x, y, z) is
 * q* = (w, -x, -y, -z)
 * 
 * For unit quaternions, inverse = conjugate.
 * 
 * @return Conjugate of this quaternion
 */
lsQuaternion lsQuaternion::conjugate() const
{
    return lsQuaternion(w, -x, -y, -z);
}

/**
 * @brief Get inverse quaternion
 * 
 * For a unit quaternion (rotation), the inverse equals
 * the conjugate since q⁻¹ = q* / |q|² and |q| = 1.
 * 
 * @return Inverse of this quaternion
 */
lsQuaternion lsQuaternion::inverse() const
{
    // True inverse is conjugate / |q|². Plain conjugate is only correct
    // for UNIT quaternions (non-unit case found by the test suite).
    float mag2 = w * w + x * x + y * y + z * z;
    if (mag2 > 0.0f)
    {
        lsQuaternion c = conjugate();
        float invMag2 = 1.0f / mag2;
        return lsQuaternion(c.w * invMag2, c.x * invMag2, c.y * invMag2, c.z * invMag2);
    }
    return conjugate();
}

//=========================================================================
// Axis-Angle Conversion
//=========================================================================

/**
 * @brief Set rotation from axis-angle
 * 
 * Converts axis-angle representation to quaternion
 * using the formula:
 * q = [cos(θ/2), sin(θ/2) * axis]
 * 
 * @param axis Rotation axis (will be normalized)
 * @param angle Rotation angle in degrees
 */
void lsQuaternion::fromAxisAngle(const lsVector3& axis, float angle)
{
    float halfAngle = angle * 0.5f * ANG2RAD;
    float s = sin(halfAngle);
    lsVector3 normalizedAxis = axis;
    normalizedAxis.normalize();
    w = cos(halfAngle);
    x = normalizedAxis.x * s;
    y = normalizedAxis.y * s;
    z = normalizedAxis.z * s;
}

//=========================================================================
// Dot Product
//=========================================================================

/**
 * @brief Calculate dot product between quaternions
 * 
 * The dot product gives cos(ω/2) where ω is the
 * angle between the rotations represented by quaternions.
 * 
 * @param other Quaternion to compute dot with
 * @return w1*w2 + x1*x2 + y1*y2 + z1*z2
 */
float lsQuaternion::dot(const lsQuaternion& other) const
{
    return w * other.w + x * other.x + y * other.y + z * other.z;
}

//=========================================================================
// Matrix Conversion
//=========================================================================

/**
 * @brief Convert quaternion to 4x4 rotation matrix
 * 
 * Converts unit quaternion to OpenGL column-major
 * 4x4 rotation matrix.
 * 
 * The conversion formula (for normalized quaternion):
 * | 1-2(y²+z²)  2(xy+zw)   2(xz-yw)   0 |
 * | 2(xy-zw)    1-2(x²+z²) 2(yz+xw)   0 |
 * | 2(xz+yw)    2(yz-xw)   1-2(x²+y²) 0 |
 * | 0           0          0          1 |
 * 
 * @param mat Output 4x4 matrix (must be initialized)
 * 
 * @note This quaternion is normalized before conversion
 */
void lsQuaternion::toMatrix4x4(lsMatrix4x4& mat) const
{
    // Ensure the quaternion is normalized for a pure rotation matrix
    lsQuaternion q = *this;
    q.normalize();

    // Precompute doubled products for efficiency
    float x2 = q.x + q.x;
    float y2 = q.y + q.y;
    float z2 = q.z + q.z;

    // Products
    float wx = q.w * x2;
    float wy = q.w * y2;
    float wz = q.w * z2;

    float xx = q.x * x2;
    float xy = q.x * y2;
    float xz = q.x * z2;

    float yy = q.y * y2;
    float yz = q.y * z2;

    float zz = q.z * z2;

    // Fill matrix (column-major)
    mat.matrix[0] = 1.0f - (yy + zz);
    mat.matrix[1] = xy + wz;
    mat.matrix[2] = xz - wy;
    mat.matrix[3] = 0.0f;

    mat.matrix[4] = xy - wz;
    mat.matrix[5] = 1.0f - (xx + zz);
    mat.matrix[6] = yz + wx;
    mat.matrix[7] = 0.0f;

    mat.matrix[8] = xz + wy;
    mat.matrix[9] = yz - wx;
    mat.matrix[10] = 1.0f - (xx + yy);
    mat.matrix[11] = 0.0f;

    mat.matrix[12] = 0.0f;
    mat.matrix[13] = 0.0f;
    mat.matrix[14] = 0.0f;
    mat.matrix[15] = 1.0f;
}
