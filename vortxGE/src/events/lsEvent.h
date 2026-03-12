/**
 * @file lsEvent.h
 * @brief Base event class for engine event system
 * 
 * lsEvent is the base class for all events in the engine.
 * Events are used for:
 * - Node deletion (deferred for safety)
 * - Ray cast hit detection
 * - Game-specific events
 * 
 * @section event_types Event Types
 * 
 * - LSET_NULL: No event / undefined
 * - LSET_DEL_NODE: Node deletion request
 * - LSET_HIT_POINT: Ray hit detection
 * 
 * @section usage Usage
 * @code
 * // Add event to queue
 * lsEvent* event = new lsEvent_DelNode();
 * event->setNode(myNode);
 * scene->Add_Event(event);
 * 
 * // Process events
 * lsEvent* e = scene->Poll_Event();
 * while (e) {
 *     e->ProcessEvent();
 *     e = scene->Poll_Event();
 * }
 * @endcode
 * 
 * @note Events are processed in the order they were added (FIFO)
 * @see lsEventManager for event queue management
 * @see lsEvent_DelNode for node deletion events
 * @see lsEvent_HitPoint for ray hit events
 */

#pragma once

//=========================================================================
// Event Type Constants
//=========================================================================

/** @brief No event / undefined */
const int LSET_NULL		= 0;
/** @brief Node deletion request */
const int LSET_DEL_NODE		= 1;
/** @brief Ray cast hit detection */
const int LSET_HIT_POINT	= 2;

// Forward declarations
class lsEventManager;
class lsNode;

/**
 * @class lsEvent
 * @brief Base class for all events
 * 
 * Events provide a way to defer actions that cannot
 * be performed immediately (like deleting nodes during
 * a frame iteration).
 * 
 * @section deferred_events Deferred Execution
 * 
 * Some operations (like deleting nodes) are not safe
 * during certain engine phases. Events defer these
 * operations to a safe time.
 */
class lsEvent
{
public:
    /**
     * @brief Default constructor
     */
	lsEvent();
	
    /**
     * @brief Destructor
     */
	virtual ~lsEvent();
	
    /**
     * @brief Process this event
     * 
     * Called when event is polled from queue.
     * Override in subclasses to implement behavior.
     */
	virtual void ProcessEvent() { /* Default implementation */ }
	
    //=========================================================================
	// Node Association
	//=========================================================================
	
    /**
     * @brief Get associated node
     * @return Pointer to node, or nullptr
     */
	lsNode *getNode() const
	{
		return node;
	}
	
    /**
     * @brief Set associated node
     * @param node Pointer to node
     */
	void setNode(lsNode *node)
	{
		this->node = node;
	}

    //=========================================================================
	// Event Manager
	//=========================================================================
	
    /**
     * @brief Get event manager
     * @return Pointer to event manager
     */
	lsEventManager *getEventmanager() const
	{
		return eventmanager;
	}

    /**
     * @brief Set event manager
     * @param eventmanager Pointer to manager
     */
	void setEventmanager(lsEventManager *eventmanager)
	{
		this->eventmanager = eventmanager;
	}

    //=========================================================================
	// Event Chain Navigation
	//=========================================================================
	
    /**
     * @brief Get next event in queue
     * @return Pointer to next event
     */
	lsEvent *getNext_event() const
	{
		return next_event;
	}

    /**
     * @brief Set next event
     * @param next_event Pointer to next event
     */
	void setNext_event(lsEvent *next_event)
	{
		this->next_event = next_event;
	}

    /**
     * @brief Get previous event in queue
     * @return Pointer to previous event
     */
	lsEvent *getPrev_event() const
	{
		return prev_event;
	}

    /**
     * @brief Set previous event
     * @param prev_event Pointer to previous event
     */
	void setPrev_event(lsEvent *prev_event)
	{
		this->prev_event = prev_event;
	}

    //=========================================================================
	// Event Type
	//=========================================================================
	
    /**
     * @brief Get event type
     * @return LSET_* constant
     */
	int getEvent_type() const
	{
		return event_type;
	}

    /**
     * @brief Set event type
     * @param event_type LSET_* constant
     */
	void setEvent_type(int event_type)
	{
		this->event_type = event_type;
	}
	
private:
    //=========================================================================
	// Private Members
	//=========================================================================
	
    /** @brief Event manager reference */
	lsEventManager *eventmanager;
	
    /** @brief Next event in queue */
	lsEvent *next_event;
    
    /** @brief Previous event in queue */
	lsEvent *prev_event;
	
    /** @brief Event type (LSET_*) */
	int event_type;
    
    /** @brief Associated node */
	lsNode* node;
};
