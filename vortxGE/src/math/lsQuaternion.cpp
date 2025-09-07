#include <cmath>

#include "lsQuaternion.h"
#include "lsVector3.h"
#include "lsMath.h"


lsQuaternion::lsQuaternion()
{
	w = 1.0;
	x = 0;
	y = 0;
	z = 0;
}

lsQuaternion::lsQuaternion(float w, float x, float y, float z)
{
	this->w = w;
	this->x = x;
	this->y = y;
	this->z = z;
}

lsQuaternion::lsQuaternion(float roll, float pitch, float yaw)
{
	float cr = cos(roll*0.5);
	float cp = cos(pitch*0.5);
	float cy = cos(yaw*0.5);

	float sr = sin(roll/2);
	float sp = sin(pitch/2);
	float sy = sin(yaw/2);

	float cpcy = cp * cy;
	float spsy = sp * sy;
	float cpsy = cp * sy;
	float spcy = sp * cy;

	w = cr * cpcy + sr * spsy;
	x = sr * cpcy - cr * spsy;
	y = cr * spcy + sr * cpsy;
	z = cr * cpsy - sr * spcy;
}

lsQuaternion::lsQuaternion(const lsVector3& axis, float angle)
{
    fromAxisAngle(axis, angle);
}

lsQuaternion::~lsQuaternion()
{
}



lsQuaternion& lsQuaternion::operator*=(const lsQuaternion& q)
{
	float E, F, G, H;

	float tw = w;
	float tx = x;
	float ty = y;

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

lsQuaternion lsQuaternion::operator*(const lsQuaternion& q2)
{
	lsQuaternion qr;

	float E, F, G, H;

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

void lsQuaternion::Set(float w, float x, float y, float z)
{
	this->w = w;
	this->x = x;
	this->y = y;
	this->z = z;
}

/*!
 * slerp(p,q,t) = (p*sin((1-t)*omega) + q*sin(t*omega)) / sin(omega)
 *
 * */
void lsQuaternion::Slerp(const lsQuaternion& to, float t)
{
	float to1[4];
	float omega, cosom, sinom;
	float scale0, scale1;

	// calc cosine
	cosom = x*to.x + y*to.y + z*to.z + w*to.w;

	// adjust signs (if necessary)
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

	// calculate coefficients
	if ((1.0 - cosom) > DELTA)
	{
		// standard case (slerp)
		omega = acos(cosom);
		sinom = sin(omega);
		scale0 = (float)sin((1.0 - t) * omega) / sinom;
		scale1 = (float)sin(t * omega) / sinom;
	} else {
		// "from" and "to" quaternions are very close
		//  ... so we can do a linear interpolation
		scale0 = 1.0f - t;
		scale1 = t;
	}

	// calculate final values
	x = scale0*x + scale1*to1[0];
	y = scale0*y + scale1*to1[1];
	z = scale0*z + scale1*to1[2];
	w = scale0*w + scale1*to1[3];
}

void lsQuaternion::Slerp(const lsQuaternion& from, const lsQuaternion& to, float t)
{
	float to1[4];
	float omega, cosom, sinom;
	float scale0, scale1;

	// calc cosine
	cosom = from.x*to.x + from.y*to.y + from.z*to.z + from.w*to.w;

	// adjust signs (if necessary)
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

	// calculate coefficients
	if ((1.0 - cosom) > DELTA)
	{
		// standard case (slerp)
		omega = acos(cosom);
		sinom = sin(omega);
		scale0 = (float)sin((1.0 - t) * omega) / sinom;
		scale1 = (float)sin(t * omega) / sinom;
	} else
	{
		// "from" and "to" quaternions are very close
		//  ... so we can do a linear interpolation
		scale0 = 1.0f - t;
		scale1 = t;
	}

	// calculate final values
	x = scale0*from.x + scale1*to1[0];
	y = scale0*from.y + scale1*to1[1];
	z = scale0*from.z + scale1*to1[2];
	w = scale0*from.w + scale1*to1[3];
}

void lsQuaternion::Lerp(const lsQuaternion& to, float t)
{
	float to1[4];
	float cosom;
	float scale0, scale1;

	// calc cosine
	cosom = x*to.x + y*to.y + z*to.z + w*to.w;

	// adjust signs (if necessary)
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

	// interpolate linearly
	scale0 = 1.0f - t;
	scale1 = t;

	// calculate final values
	x = scale0*x + scale1*to1[0];
	y = scale0*y + scale1*to1[1];
	z = scale0*z + scale1*to1[2];
	w = scale0*w + scale1*to1[3];
}

float lsQuaternion::magnitude() const
{
    return sqrt(w * w + x * x + y * y + z * z);
}

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

lsQuaternion lsQuaternion::conjugate() const
{
    return lsQuaternion(w, -x, -y, -z);
}

lsQuaternion lsQuaternion::inverse() const
{
    // For a unit quaternion, the inverse is the conjugate.
    // Assuming this quaternion represents a rotation, it should be a unit quaternion.
    return conjugate();
}

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

float lsQuaternion::dot(const lsQuaternion& other) const
{
    return w * other.w + x * other.x + y * other.y + z * other.z;
}

void lsQuaternion::toMatrix4x4(lsMatrix4x4& mat) const
{
    // Ensure the quaternion is normalized for a pure rotation matrix
    lsQuaternion q = *this;
    q.normalize();

    float x2 = q.x + q.x;
    float y2 = q.y + q.y;
    float z2 = q.z + q.z;

    float wx = q.w * x2;
    float wy = q.w * y2;
    float wz = q.w * z2;

    float xx = q.x * x2;
    float xy = q.x * y2;
    float xz = q.x * z2;

    float yy = q.y * y2;
    float yz = q.y * z2;

    float zz = q.z * z2;

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
