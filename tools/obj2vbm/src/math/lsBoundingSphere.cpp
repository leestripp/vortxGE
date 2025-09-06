#include <cmath>

#include "lsBoundingSphere.h"

lsBoundingSphere::lsBoundingSphere()
{
	radius = 1.0f;

}

lsBoundingSphere::~lsBoundingSphere()
{
}



float lsBoundingSphere::distance( lsVector3 V1 )
{
	float dist;
	lsVector3 V3;

	V3 = center - V1;
	dist = sqrt( V3.x*V3.x + V3.y*V3.y + V3.z*V3.z );

	return dist;
}
