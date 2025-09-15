#pragma once

#define lsPI 3.14159265358979323846
#define ANG2RAD 3.14159265358979323846/180.0
#define RAD2ANG 180.0/3.14159265358979323846

// array types
typedef float	Vector2f[2];
typedef float	Vector3f[3];
typedef float	Vector4f[4];
typedef float	Matrix44f[16];

// Classes used
class lsVector2;
class lsVector3;
class lsVector4;

/*!
 * \brief The lsMath class
 *
 * Commonly used math functions for 3D space.\n
 *
 */
class lsMath
{
public:
	lsMath();
	virtual ~lsMath();
	
	bool approxEqual( float a, float b, float epsilon );
	
	lsVector3 Cross( const lsVector3& Vec1, const lsVector3& Vec2 );
	float Dot( const lsVector3& Vec1, const lsVector3& Vec2 );
};
