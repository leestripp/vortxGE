#include "lsVector2.h"

lsVector2::lsVector2()
{
	x = 0.0f;
	y = 0.0f;
}

lsVector2::lsVector2( float v1, float v2 )
{
	x = v1;
	y = v2;
}

lsVector2::~lsVector2()
{
}


lsVector2 lsVector2::operator+(const lsVector2 &v)
{
	lsVector2 r;

	r.x = x + v.x;
	r.y = y + v.y;

	return r;
}

lsVector2 lsVector2::operator-(const lsVector2 &v)
{
	lsVector2 r;

	r.x = x - v.x;
	r.y = y - v.y;

	return r;
}


lsVector2 lsVector2::operator*(const lsVector2 &v)
{
	lsVector2 r;

	r.x = x * v.x;
	r.y = y * v.y;

	return r;
}

lsVector2 lsVector2::operator*(float t)
{
	lsVector2 r;

	r.x = x * t;
	r.y = y * t;

	return r;
}

lsVector2 lsVector2::operator/(float t)
{
	lsVector2 r;

	r.x = x / t;
	r.y = y / t;

	return r;
}


