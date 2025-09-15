#include "lsVector2.h"

lsVector2::lsVector2()
{
	x = 0.0;
	y = 0.0;
}

lsVector2::lsVector2( float v1, float v2 )
{
	x = v1;
	y = v2;
}

// ***************
// ***** Methods
// ***************

lsVector2 lsVector2::operator+(const lsVector2 &v) const
{
	lsVector2 r;

	r.x = x + v.x;
	r.y = y + v.y;

	return r;
}

lsVector2 lsVector2::operator-(const lsVector2 &v) const
{
	lsVector2 r;

	r.x = x - v.x;
	r.y = y - v.y;

	return r;
}

lsVector2 lsVector2::operator*(const lsVector2 &v) const
{
	lsVector2 r;

	r.x = x * v.x;
	r.y = y * v.y;

	return r;
}

lsVector2 lsVector2::operator*(float t) const
{
	lsVector2 r;
	
	r.x = x * t;
	r.y = y * t;
	
	return r;
}

lsVector2 lsVector2::operator/( float t ) const
{
	lsVector2 res;
	
	res.x = x / t;
	res.y = y / t;
	
	return res;
}

const string& lsVector2::toString()
{
	stringstream ss;
	
	ss << "(";
	ss << x;
	ss << ",";
	ss << y;
	ss << ")";
	output = ss.str();
	
	return output;
}
