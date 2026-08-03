#include <sstream>

#include "lsVector3.h"


lsVector3::lsVector3()
{
	x = 0;
	y = 0;
	z = 0;
}

lsVector3::lsVector3( float XX, float YY, float ZZ )
{
	x = XX;
	y = YY;
	z = ZZ;
}

// ***************
// ***** Methods
// ***************

lsVector3 lsVector3::operator+(float s) const
{
	lsVector3 result;

	result.x = x + s;
	result.y = y + s;
	result.z = z + s;

	return result;
}

lsVector3 lsVector3::operator-() const
{
	lsVector3 result;

	result.x = -x;
	result.y = -y;
	result.z = -z;

	return result;
}

lsVector3 lsVector3::operator+(const lsVector3 &v) const
{
	lsVector3 result;

	result.x = x + v.x;
	result.y = y + v.y;
	result.z = z + v.z;

	return result;
}

lsVector3 lsVector3::operator-(float s) const
{
	lsVector3 result;

	result.x = x - s;
	result.y = y - s;
	result.z = z - s;

	return result;
}

lsVector3 lsVector3::operator-(const lsVector3 &v) const
{
	lsVector3 result;

	result.x = x - v.x;
	result.y = y - v.y;
	result.z = z - v.z;

	return result;
}

lsVector3 lsVector3::operator*(float t) const
{
	lsVector3 result;

	result.x = x * t;
	result.y = y * t;
	result.z = z * t;

	return result;
}

lsVector3& lsVector3::operator+=(const lsVector3 &v)
{
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}

lsVector3& lsVector3::operator-=(const lsVector3 &v)
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
}

lsVector3 lsVector3::operator*(const lsVector3 &v) const
{
	lsVector3 result;

	// Note: This operator is currently implemented as a cross product.
	result.x = y * v.z - z * v.y;
	result.y = z * v.x - x * v.z;
	result.z = x * v.y - y * v.x;

	return result;
}

// Implementation of the Cross product function
lsVector3 lsVector3::Cross(const lsVector3 &v) const
{
	lsVector3 result;
	
	// Cross product: (a2*b3 - a3*b2, a3*b1 - a1*b3, a1*b2 - a2*b1)
	result.x = y * v.z - z * v.y;
	result.y = z * v.x - x * v.z;
	result.z = x * v.y - y * v.x;

	return result;
}

// Implementation of the Dot product function
float lsVector3::Dot(const lsVector3 &v) const
{
    // Dot product: (a1*b1 + a2*b2 + a3*b3)
    return (x * v.x + y * v.y + z * v.z);
}

lsVector3 lsVector3::operator/(float t) const
{
	lsVector3 result;

	float f = 1.0 / t;
	result.x = x * f;
	result.y = y * f;
	result.z = z * f;

	return result;
}

lsVector3& lsVector3::RotateAboutX(float angle)
{
	float s = sin(angle);
	float c = cos(angle);

	float ny = c * y - s * z;
	float nz = c * z + s * y;

	y = ny;
	z = nz;

	return *this;
}

lsVector3& lsVector3::RotateAboutY( float angle )
{
	float s = sin(angle);
	float c = cos(angle);

	float nx = c * x + s * z;
	float nz = c * z - s * x;

	x = nx;
	z = nz;

	return *this;
}

lsVector3& lsVector3::RotateAboutZ(float angle)
{
	float s = sin(angle);
	float c = cos(angle);

	float nx = c * x - s * y;
	float ny = c * y + s * x;

	x = nx;
	y = ny;

	return *this;
}

lsVector3& lsVector3::RotateAboutAxis(float angle, const lsVector3& axis)
{
	float s = sin(angle);
	float c = cos(angle);
	float k = 1.0 - c;

	float nx = x * (c + k * axis.x * axis.x) + y * (k * axis.x * axis.y - s * axis.z)
	+ z * (k * axis.x * axis.z + s * axis.y);
	float ny = x * (k * axis.x * axis.y + s * axis.z) + y * (c + k * axis.y * axis.y) + z * (k * axis.y * axis.z - s * axis.x);
	float nz = x * (k * axis.x * axis.z - s * axis.y) + y * (k * axis.y * axis.z + s * axis.x) + z * (c + k * axis.z * axis.z);

	x = nx;
	y = ny;
	z = nz;

	return *this;
}

float lsVector3::length()
{
    float l;

    l = (x*x) + (y*y) + (z*z);

    return( (float)sqrt( l ) );
}

lsVector3& lsVector3::normalize()
{
	// Zero-length guard, consistent with lsVector4::normalize() and
	// lsQuaternion::normalize() (was missing — found by the test suite)
	float len = sqrt(x * x + y * y + z * z);
	if (len)
		*this = *this / len;
	return *this;
}

void lsVector3::copy(const lsVector3 &v)
{
	x = v.x;
	y = v.y;
	z = v.z;
}

void lsVector3::set(float x,float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

void lsVector3::set( const lsVector3& val )
{
	this->x = val.x;
	this->y = val.y;
	this->z = val.z;
}

void lsVector3::add(const lsVector3& vec)
{
	x += vec.x;
	y += vec.y;
	z += vec.z;

}


lsVector3 lsVector3::scalarMult(float a)
{
	lsVector3 res;

	res.x = x * a;
	res.y = y * a;
	res.z = z * a;

	return res;
}

float lsVector3::distance( const lsVector3 &V1 )
{
	float dist;
	lsVector3 V3;
	lsVector3 t(x,y,z);

	V3 = t - V1;
	dist = sqrt( V3.x*V3.x + V3.y*V3.y + V3.z*V3.z );

	return dist;
}


const string& lsVector3::toString()
{
	stringstream ss;

	ss << "(";
	ss << x;
	ss << ",";
	ss << y;
	ss << ",";
	ss << z;
	ss << ")";
	output = ss.str();

	return output;
}

// ***************
// ***** Debug
// ***************

void lsVector3::debug(const string& msg)
{
    cout << msg;
	cout << "Vector3" << toString() << endl;
}

