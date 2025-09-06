#ifndef LSMATH_H
#define LSMATH_H

#define lsPI 3.14159265358979323846
#define ANG2RAD 3.14159265358979323846/180.0
#define RAD2ANG 180.0/3.14159265358979323846

// Classes used
class lsVector3;

//! 3D Math functions
/*!
 * Commonly used math functions for 3D space.
 *
 * */
class lsMath
{
public:
    lsMath();
    virtual ~lsMath();

    lsVector3 Cross( const lsVector3& Vec1, const lsVector3& Vec2 );
    float Dot( const lsVector3& Vec1, const lsVector3& Vec2 );

};

#endif // LSMATH_H
