#include "lsMath.h"

#include "lsVector2.h"
#include "lsVector3.h"


lsMath::lsMath()
{

}

lsMath::~lsMath()
{
}

lsVector3 lsMath::Cross(const lsVector3& Vec1, const lsVector3& Vec2)
{
	lsVector3 res;

	res.x = Vec1.y * Vec2.z - Vec1.z * Vec2.y;
	res.y = Vec1.z * Vec2.x - Vec1.x * Vec2.z;
	res.z = Vec1.x * Vec2.y - Vec1.y * Vec2.x;

	return res;
}

float lsMath::Dot( const lsVector3& Vec1, const lsVector3& Vec2 )
{
	float res;

	res = Vec1.x * Vec2.x + Vec1.y * Vec2.y + Vec1.z * Vec2.z;

	return res;
}
