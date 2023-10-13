/*
 * Copyright (c) 2016 by Lee A. Stripp <leestripp@gmail.com>
 *
 * This file is part of the vortxGE library.
 *
 * vortxGE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * vortxGE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with vortxGE.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef LSEVENT_H_
#define LSEVENT_H_

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

#endif /* LSEVENT_H_ */
