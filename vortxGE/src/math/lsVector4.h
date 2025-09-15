#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <cmath>

// array types
typedef float	Vector4f[4];

using namespace std;

class lsVector4
{
public:
	lsVector4();
	lsVector4( float XX, float YY, float ZZ, float WW );
	
	lsVector4 operator+(float s) const;
	lsVector4 operator+(const lsVector4 &v) const;
	lsVector4 operator-(const lsVector4 &v) const;
	lsVector4 operator*(const lsVector4 &v) const;
	lsVector4 operator*(float t) const;
	lsVector4 operator/(float t) const;
	lsVector4 operator-() const;
	
	float length();
	void normalize();

	void copy(const lsVector4 &v);
	void set(float x,float y, float z, float w );
	float distance( const lsVector4 &V1 );

    const string& toString();
    void debug(const string& msg);

	float getX() const
	{
		return x;
	}

	void setX(float x)
	{
		this->x = x;
	}

	float getY() const
	{
		return y;
	}

	void setY(float y)
	{
		this->y = y;
	}

	float getZ() const
	{
		return z;
	}

	void setZ(float z)
	{
		this->z = z;
	}

	float getW() const
	{
		return w;
	}

	void setW(float w)
	{
		this->w = w;
	}
	
	float x;
	float y;
	float z;
	float w;
	
private:
    string output;
};
