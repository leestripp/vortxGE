#include "lsVector4.h"

lsVector4::lsVector4()
{
	x = 0;
	y = 0;
	z = 0;
	w = 0;
}

lsVector4::lsVector4( float XX, float YY, float ZZ, float WW )
{
	x = XX;
	y = YY;
	z = ZZ;
	w = WW;

}

// ***************
// ***** Methods
// ***************

lsVector4 lsVector4::operator+(float s) const
{
    lsVector4 res;
	
    res.x = x + s;
    res.y = y + s;
    res.z = z + s;
    res.w = w;
	
    return res;
}

lsVector4 lsVector4::operator+(const lsVector4 &v) const
{
	lsVector4 res;
	
	res.x = x + v.x;
	res.y = y + v.y;
	res.z = z + v.z;
	res.w = w;
	
	return res;
}

lsVector4 lsVector4::operator-(const lsVector4 &v) const
{
	lsVector4 res;

	res.x = x - v.x;
	res.y = y - v.y;
	res.z = z - v.z;
	res.w = w;

	return res;
}

lsVector4 lsVector4::operator-() const
{
	lsVector4 res;

	res.x = -x;
	res.y = -y;
	res.z = -z;
	res.w = w;

	return res;
}


lsVector4 lsVector4::operator*(const lsVector4 &v) const
{
	lsVector4 res;

	res.x = y * v.z - z * v.y;
	res.y = z * v.x - x * v.z;
	res.z = x * v.y - y * v.x;
	res.w = w;

	return res;
}

lsVector4 lsVector4::operator*(float t) const
{
	lsVector4 res;

	res.x = x * t;
	res.y = y * t;
	res.z = z * t;
	res.w = w;

	return res;
}


lsVector4 lsVector4::operator/(float t) const
{
	lsVector4 res;

	res.x = x / t;
	res.y = y / t;
	res.z = z / t;
	res.w = w;

	return res;
}



float lsVector4::length()
{
    float l;

    l = (x*x) + (y*y) + (z*z);

    return( (float)sqrt( l ) );
}

void lsVector4::normalize()
{
	float len;

	len = length();
	if (len)
	{
		x = x / len;
		y = y / len;
		z = z / len;
	}
}

void lsVector4::copy(const lsVector4 &v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	w = v.w;

}

void lsVector4::set(float x,float y, float z, float w )
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;

}

float lsVector4::distance( const lsVector4 &V1 )
{
	float dist;
	lsVector4 V3;
	lsVector4 t(x,y,z,w);

	V3 = t - V1;
	dist = sqrt( V3.x*V3.x + V3.y*V3.y + V3.z*V3.z );

	return dist;
}

/*!
 Saveing data to disk
 */
const string& lsVector4::toString()
{
	stringstream ss;

	ss << "(";
	ss << x;
	ss << ",";
	ss << y;
	ss << ",";
	ss << z;
	ss << ",";
	ss << w;
	ss << ")";
	output = ss.str();

	return output;
}

// ***************
// ***** Debug
// ***************

void lsVector4::debug( const string& msg )
{
	cout << msg;
	cout << " Vector4(" << x << "," << y << "," << z << "," << w << ") " << endl;
}
