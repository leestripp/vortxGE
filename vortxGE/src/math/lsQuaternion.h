#pragma once

#include "lsMatrix4x4.h"

#define DELTA 1e-10     // error tolerance

class lsVector3; // Forward declaration

class lsQuaternion
{
public:
	lsQuaternion();
	lsQuaternion(float w, float x, float y, float z);
	lsQuaternion(float roll, float pitch, float yaw);
    lsQuaternion(const lsVector3& axis, float angle); // from axis-angle
	virtual ~lsQuaternion();

	lsQuaternion& operator*=( const lsQuaternion& q );
	lsQuaternion operator*( const lsQuaternion& q2 );

	void Set(float w, float x, float y, float z);
	void Slerp(const lsQuaternion &to, float t);
	void Slerp(const lsQuaternion &from, const lsQuaternion &to, float t);
	void Lerp(const lsQuaternion &to, float t);

    float magnitude() const;
    void normalize();
    lsQuaternion conjugate() const;
    lsQuaternion inverse() const;
    void fromAxisAngle(const lsVector3& axis, float angle);
    float dot(const lsQuaternion& other) const;
    void toMatrix4x4(lsMatrix4x4& mat) const;

	// Data
	float w, x, y, z;
};
