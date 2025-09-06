#ifndef LSPLANE_H_
#define LSPLANE_H_

#include <iostream>

#include "lsVector3.h"

using namespace std;

//! Plane math class
/*!
 * Holds a point and normal vector
 *
 */
class lsPlane
{
public:
	lsPlane();
	lsPlane( lsVector3 &v1, lsVector3 &v2, lsVector3 &v3 );
	virtual ~lsPlane();


	void set3Points( lsVector3 &v1,  lsVector3 &v2,  lsVector3 &v3);
	void setNormalAndPoint( const lsVector3 &normal, const lsVector3 &point);
	void setCoefficients( float a, float b, float c, float d );
	float distance(const lsVector3 &p );

	void debug();


	lsVector3 normal;
	lsVector3 point;
	float d;

};

#endif /* LSPLANE_H_ */
