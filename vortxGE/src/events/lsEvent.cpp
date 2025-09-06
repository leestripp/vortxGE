#include "lsEventManager.h"
#include "lsEvent.h"

lsEvent::lsEvent()
{
	next_event = 0;
	prev_event = 0;

	event_type = LSET_NULL;

}

lsEvent::~lsEvent()
{
}

