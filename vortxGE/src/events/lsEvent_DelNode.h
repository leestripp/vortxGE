/**
 * @file lsEvent_DelNode.h
 * @brief Event for deferred node deletion
 * 
 * lsEvent_DelNode represents a request to delete a scene node.
 * Node deletion is deferred because:
 * - Cannot delete while iterating over node list
 * - Allows proper cleanup before frame ends
 * - Prevents crashes during rendering
 * 
 * @section usage Usage
 * @code
 * // Schedule node for deletion (safe)
 * lsEvent_DelNode* event = new lsEvent_DelNode();
 * event->setNode(nodeToDelete);
 * scene->Add_Event(event);
 * 
 * // Event will be processed at end of frame
 * @endcode
 * 
 * @note Deleted nodes are actually removed in lsNode::CleanupDeletedNodes()
 * @see lsEvent for base event class
 * @see lsNode::MarkForDeletion() for marking nodes
 */

#pragma once

#include "lsEvent.h"

/**
 * @class lsEvent_DelNode
 * @brief Event for safe node deletion
 * 
 * Schedules a node for deletion at a safe time.
 * The node will be removed from its parent during
 * the cleanup phase.
 */
class lsEvent_DelNode : public lsEvent
{
public:
    /**
     * @brief Default constructor
     */
	lsEvent_DelNode();
	
    /**
     * @brief Destructor
     */
	virtual ~lsEvent_DelNode();
	
    /**
     * @brief Process the deletion event
     * 
     * Marks the associated node for deletion.
     * Actual removal happens during CleanupDeletedNodes().
     */
	void ProcessEvent() override;
};
