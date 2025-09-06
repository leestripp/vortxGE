#ifndef LSEVENT_HITPOINT_H
#define LSEVENT_HITPOINT_H

#include "../math/lsVector3.h"
#include "lsEvent.h"


class lsNode;

//! Ray Cast hit point event
/*!
 * This event is created when a Ray hits an object.
 *
 * */
class lsEvent_HitPoint : public lsEvent
{
public:
	lsEvent_HitPoint();
	virtual ~lsEvent_HitPoint();

	void Set_HitNode( lsNode* node );
	void Set_HitPoint( float x, float y, float z );
	void Set_HitNormal( float x, float y, float z );

private:
	lsNode* hit_node;
	lsVector3 hit_point;
	lsVector3 hit_normal;

};

#endif // LSEVENT_HITPOINT_H
