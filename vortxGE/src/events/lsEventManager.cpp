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

#include "../vortx_debug.h"
#include "../vortxGE.h"

// events
#include "lsEvent.h"

#include "lsEventManager.h"


lsEventManager::lsEventManager()
{
	event_list = NULL;
	last_event = NULL;
	current_event = NULL;

}

lsEventManager::~lsEventManager()
{
	// Clean up
    while( Poll_Event() ) {}
	if( current_event ) delete current_event;
}

void lsEventManager::Add_Event( lsEvent *e )
{
	if( last_event )
	{
		last_event->setNext_event( e );
		last_event = e;

	} else
	{
		event_list = e;
		last_event = e;
		current_event = NULL;
	}
}

lsEvent *lsEventManager::Poll_Event()
{
	if( current_event )
	{
		delete current_event;
		current_event = NULL;
	}

	if( event_list )
	{
		current_event = event_list;
		event_list = event_list->getNext_event();
		if(! event_list ) last_event = NULL;
		current_event->setNext_event( NULL );

	} else
	{
		last_event = NULL;
		current_event = NULL;
	}

	return current_event;
}




