#pragma once

#include <sstream>
#include <string>

// array types
typedef float	Vector2f[2];

using namespace std;


class lsVector2
{
public:
	lsVector2();
	lsVector2( float v1, float v2 );
	
	lsVector2 operator+(const lsVector2 &v) const;
	lsVector2 operator-(const lsVector2 &v) const;
	lsVector2 operator*(const lsVector2 &v) const;
	lsVector2 operator*(float t) const;
	lsVector2 operator/(float t) const;
	
    const string& toString();
	
	float x;
	float y;
	
private:
    string output;
};
