#ifndef LSVECTOR3_H_
#define LSVECTOR3_H_

#include <iostream>
#include <cmath>

// array types
typedef float	Vector3f[3];


using namespace std;

//! Position, Vectors
/*!
 * 3D vector class with functions.
 *
 */
class lsVector3
{
public:
    lsVector3();
	lsVector3( float XX, float YY, float ZZ );
	
    lsVector3 operator+(float s);
    lsVector3 operator+(const lsVector3 &v);
    lsVector3 operator-(float s);
    lsVector3 operator-(const lsVector3 &v);
    lsVector3 operator*(float t);
    lsVector3 operator*(const lsVector3 &v);
    lsVector3 operator/(float t);

    lsVector3& RotateAboutX(float angle);
    lsVector3& RotateAboutY(float angle);
    lsVector3& RotateAboutZ(float angle);
    lsVector3& RotateAboutAxis(float angle, const lsVector3& axis);

    void set( const lsVector3& val );
    float length();
    lsVector3& normalize();

    void copy(const lsVector3 &v);
    void set(float x,float y, float z);

    void add( const lsVector3& vec );
    lsVector3 scalarMult(float a);
    float distance( const lsVector3 &V1 );

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


    float x;
    float y;
    float z;

private:
    string output;

};

#endif /* LSVECTOR3_H_ */
