#ifndef LSBOUNDINGSPHERE_H_
#define LSBOUNDINGSPHERE_H_

#include "lsVector3.h"


//! Simple Bounding Sphere
/*!
 * Used for culling, Physics, collision etc...
 *
 */
class lsBoundingSphere
{
public:
	lsBoundingSphere();
	virtual ~lsBoundingSphere();

	float distance( const lsVector3& V1 );

	void draw();

	// Data
	lsVector3	center;
	float		radius;

};

#endif /* LSBOUNDINGSPHERE_H_ */
