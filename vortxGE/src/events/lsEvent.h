#pragma once

// Core event tupes
const int LSET_NULL		= 0;
const int LSET_DEL_NODE		= 1;
const int LSET_HIT_POINT	= 2;

class lsEventManager;
class lsNode;


//! Base Event
/*!
 * Base for game events
 *
 */
class lsEvent
{
public:
	lsEvent();
	virtual ~lsEvent();
	
	virtual void ProcessEvent() { /* Default implementation */ }
	
	lsNode *getNode() const
	{
		return node;
	}
	
	void setNode(lsNode *node)
	{
		this->node = node;
	}

	lsEventManager *getEventmanager() const
	{
		return eventmanager;
	}

	void setEventmanager(lsEventManager *eventmanager)
	{
		this->eventmanager = eventmanager;
	}

	// Event navi
	lsEvent *getNext_event() const
	{
		return next_event;
	}

	void setNext_event(lsEvent *next_event)
	{
		this->next_event = next_event;
	}

	lsEvent *getPrev_event() const
	{
		return prev_event;
	}

	void setPrev_event(lsEvent *prev_event)
	{
		this->prev_event = prev_event;
	}

	int getEvent_type() const
	{
		return event_type;
	}

	void setEvent_type(int event_type)
	{
		this->event_type = event_type;
	}
	
private:
	lsEventManager *eventmanager;
	
	lsEvent *next_event;
	lsEvent *prev_event;
	
	int event_type;
	// Node events
	lsNode* node;
};
