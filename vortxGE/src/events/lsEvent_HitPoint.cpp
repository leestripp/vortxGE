#include "lsEvent_HitPoint.h"


lsEvent_HitPoint::lsEvent_HitPoint()
{
	setEvent_type( LSET_HIT_POINT );

	hit_node = NULL;

}

lsEvent_HitPoint::~lsEvent_HitPoint()
{
}

void lsEvent_HitPoint::Set_HitNode(lsNode* node)
{
	hit_node = node;
}

void lsEvent_HitPoint::Set_HitNormal(float x, float y, float z)
{
	hit_normal.x = x;
	hit_normal.y = y;
	hit_normal.z = z;
}

void lsEvent_HitPoint::Set_HitPoint(float x, float y, float z)
{
	hit_point.x = x;
	hit_point.y = y;
	hit_point.z = z;
}
