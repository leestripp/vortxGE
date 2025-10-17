#include "../vortx_debug.h"
#include "../vortxGE.h"

// events
#include "lsEvent.h"
#include "lsEvent_DelNode.h"

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
