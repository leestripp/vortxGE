#ifndef LSVECTOR2_H_
#define LSVECTOR2_H_

#include <sstream>
#include <string>

// array types
typedef float	Vector2f[2];

using namespace std;

//! 2D Point
/*!
 * Storage and math functions
 *
 */
class lsVector2
{
public:
	lsVector2();
	lsVector2( float v1, float v2 );
	
	lsVector2 operator+(const lsVector2 &v);
	lsVector2 operator-(const lsVector2 &v);
	lsVector2 operator*(const lsVector2 &v);
	lsVector2 operator*(float t);
	lsVector2 operator/(float t);
	
    const string& toString();
	
	float x;
	float y;
	
private:
    string output;
};

#endif /* LSVECTOR2_H_ */
