/**
 * @file lsEventManager.h
 * @brief Event queue manager for deferred event processing
 * 
 * lsEventManager handles the event queue for the engine.
 * Events are added to the queue and processed in order,
 * providing thread-safe deferred execution.
 * 
 * @section usage Usage
 * @code
 * // Add event to queue
 * lsEvent* event = new lsEvent_DelNode();
 * eventManager->Add_Event(event);
 * 
 * // Process all events
 * lsEvent* e = eventManager->Poll_Event();
 * while (e) {
 *     e->ProcessEvent();
 *     e = eventManager->Poll_Event();
 * }
 * @endcode
 * 
 * @see lsEvent for base event class
 * @see lsScene for scene-level event handling
 */

#ifndef LSEVENTMANAGER_H_
#define LSEVENTMANAGER_H_

// Forward declarations
class lsScene;
class lsEvent;

/**
 * @class lsEventManager
 * @brief Manages event queue for deferred processing
 * 
 * Provides FIFO event queue for deferred operations.
 * Events are processed in the order they were added.
 * 
 * @section queue Queue
 * 
 * Events are stored in a linked list:
 * - Add_Event(): Adds to end of queue
 * - Poll_Event(): Removes from front of queue
 */
class lsEventManager
{
public:
    /**
     * @brief Default constructor
     */
	lsEventManager();
	
    /**
     * @brief Destructor
     * 
     * Cleans up any remaining events in queue.
     */
	virtual ~lsEventManager();

    /**
     * @brief Add event to queue
     * 
     * Appends event to end of processing queue.
     * 
     * @param e Pointer to event to add
     */
	void Add_Event( lsEvent* e );
    
    /**
     * @brief Get next event from queue
     * 
     * Removes and returns the oldest event from queue.
     * 
     * @return Pointer to next event, or nullptr if queue empty
     */
    lsEvent* Poll_Event();

private:
    //=========================================================================
	// Private Members
	//=========================================================================
	
    /** @brief Head of event queue */
	lsEvent* event_list;
    
    /** @brief Tail of event queue */
	lsEvent* last_event;
    
    /** @brief Current event being processed */
	lsEvent* current_event;

};

#endif /* LSEVENTMANAGER_H_ */
