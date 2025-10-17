#include "lsEvent_DelNode.h"

#include "../scene/lsNode.h"

lsEvent_DelNode::lsEvent_DelNode()
{
	setEvent_type( LSET_DEL_NODE );
	
	setNode( 0 );
}

lsEvent_DelNode::~lsEvent_DelNode()
{
}

// Override to mark node for deletion
void lsEvent_DelNode::ProcessEvent()
{
	if(getNode())
	{
		// Mark the node for deletion
		getNode()->MarkForDeletion();
	}
}
