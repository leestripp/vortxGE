#include "lsEvent_DelNode.h"


lsEvent_DelNode::lsEvent_DelNode()
{
	setEvent_type( LSET_DEL_NODE );

	setNode( 0 );
}

lsEvent_DelNode::~lsEvent_DelNode()
{
}

