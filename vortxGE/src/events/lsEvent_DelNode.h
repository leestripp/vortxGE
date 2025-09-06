#ifndef LSEVENT_DELNODE_H
#define LSEVENT_DELNODE_H

#include "lsEvent.h"


//! Delete Scene node
/*!
 * Event to delete a scene node, its not safe to delete a node directly at any time so add this event insted.
 *
 * */
class lsEvent_DelNode : public lsEvent
{
public:
	lsEvent_DelNode();
	virtual ~lsEvent_DelNode();

};

#endif // LSEVENT_DELNODE_H
